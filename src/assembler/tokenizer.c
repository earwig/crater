/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdlib.h>
#include <string.h>

#include "tokenizer.h"
#include "directives.h"
#include "parse_util.h"
#include "../logging.h"
#include "../rom.h"

/* Sentinel values for overlap table */
const ASMLine header_sentinel, bounds_sentinel;

/*
    Add a given line, representing a label, to the symbol table.

    Return NULL on success and an ErrorInfo object on failure (in the case of
    duplicate labels).
*/
static ErrorInfo* add_label_to_table(
    ASMSymbolTable *symtable, const ASMLine *line, size_t offset, ssize_t slot)
{
    char *symbol = strndup(line->data, line->length - 1);
    if (!symbol)
        OUT_OF_MEMORY()

    const ASMSymbol *current = asm_symtable_find(symtable, symbol);
    if (current) {
        ErrorInfo *ei = error_info_create(line, ET_SYMBOL, ED_SYM_DUPE_LABELS);
        error_info_append(ei, current->line);
        return ei;
    }

    ASMSymbol *label = malloc(sizeof(ASMSymbol));
    if (!label)
        OUT_OF_MEMORY()

    size_t block_offset = offset & 0x3FFF;
    label->offset = slot >= 0 ? (block_offset + slot * 0x4000) :
                    (offset >= 0xC000 ? (block_offset + 0x8000) : offset);
    label->symbol = symbol;
    label->line = line;
    asm_symtable_insert(symtable, label);
    return NULL;
}

/*
    Handle an origin directive by updating the offset and (maybe) the slot.

    Return NULL on success and an ErrorInfo object on failure.
*/
static ErrorInfo* handle_origin_directive(
    const ASMLine *line, size_t *offset, ssize_t *slot)
{
    if (!DIRECTIVE_HAS_ARG(line, DIR_ORIGIN))
        return error_info_create(line, ET_PREPROC, ED_PP_NO_ARG);

    uint32_t arg;
    if (!dparse_uint32_t(&arg, line, DIR_ORIGIN))
        return error_info_create(line, ET_PREPROC, ED_PP_BAD_ARG);

    *offset = arg;
    // TODO: if different block, *slot <-- slot lookup table for this block
    return NULL;
}

/*
    Handle a block directive by updating the offset and slot.

    Return NULL on success and an ErrorInfo object on failure.
*/
static ErrorInfo* handle_block_directive(
    const ASMLine *line, size_t *offset, ssize_t *slot)
{
    if (!DIRECTIVE_HAS_ARG(line, DIR_BLOCK))
        return error_info_create(line, ET_PREPROC, ED_PP_NO_ARG);

    uint8_t *args;
    size_t dir_offset = DIRECTIVE_OFFSET(line, DIR_BLOCK) + 1, nargs;

    if (!parse_bytes(&args, &nargs, line->data + dir_offset,
                     line->length - dir_offset))
        return error_info_create(line, ET_PREPROC, ED_PP_BAD_ARG);

    if (nargs < 1 || nargs > 2 || args[0] >= 64 || (nargs == 2 && args[1] > 2))
        return error_info_create(line, ET_PREPROC, ED_PP_BAD_ARG);

    if (nargs == 2 && args[0] == 0 && args[1] != 0)
        return error_info_create(line, ET_LAYOUT, ED_LYT_BLOCK0);

    *offset = args[0] * (16 << 10);
    *slot = nargs == 2 ? args[1] : -1;
    free(args);
    return NULL;
}

/*
    Parse data encoded in a line into an ASMData object.

    On success, return NULL and store the instruction in *data_ptr. On failure,
    return an ErrorInfo object; *data_ptr is not modified.
*/
static ErrorInfo* parse_data(
    const ASMLine *line, ASMData **data_ptr, size_t offset)
{
    // TODO
    DEBUG("parse_data(): %.*s", (int) line->length, line->data)

    return error_info_create(line, ET_PARSER, ED_PARSE_SYNTAX);
}

/*
    Parse an instruction encoded in a line into an ASMInstruction object.

    On success, return NULL and store the instruction in *inst_ptr. On failure,
    return an ErrorInfo object; *inst_ptr is not modified.
*/
static ErrorInfo* parse_instruction(
    const ASMLine *line, ASMInstruction **inst_ptr, size_t offset)
{
    // TODO
    DEBUG("parse_instruction(): %.*s", (int) line->length, line->data)

    return error_info_create(line, ET_PARSER, ED_PARSE_SYNTAX);
}

/*
    Check if the given location overlaps with any existing objects.

    On success, return NULL and add the location to the overlap table.
    On failure, return an ErrorInfo object.
*/
static ErrorInfo* check_layout(
    const ASMLine **overlap_table, size_t size, const ASMLocation *loc,
    const ASMLine *line, const ASMLine *origin)
{
    // TODO: never let boundaries cross without state->cross_blocks
    const ASMLine *clash = NULL;

    if (loc->offset + loc->length > size) {
        clash = &bounds_sentinel;
    } else {
        for (size_t i = 0; i < loc->length; i++) {
            if (overlap_table[loc->offset + i]) {
                clash = overlap_table[loc->offset + i];
                break;
            }
        }
    }

    if (clash) {
        ErrorInfo *ei = error_info_create(line, ET_LAYOUT,
            (clash == &header_sentinel) ? ED_LYT_OVERLAP_HEAD :
            (clash == &bounds_sentinel) ? ED_LYT_BOUNDS : ED_LYT_OVERLAP);

        if (origin)
            error_info_append(ei, origin);
        if (clash != &header_sentinel && clash != &bounds_sentinel)
            error_info_append(ei, clash);
        return ei;
    }

    for (size_t i = 0; i < loc->length; i++)
        overlap_table[loc->offset + i] = line;
    return NULL;
}

/*
    Tokenize ASMLines into ASMInstructions and ASMData.

    NULL is returned on success and an ErrorInfo object is returned on failure.
    state->instructions, state->data, and state->symtable may or may not be
    modified regardless of success.
*/
ErrorInfo* tokenize(AssemblerState *state)
{
    size_t size = state->rom_size ? state->rom_size : ROM_SIZE_MAX;
    const ASMLine **overlap_table = calloc(size, sizeof(const ASMLine*));
    if (!overlap_table)
        OUT_OF_MEMORY()

    ErrorInfo *ei = NULL;
    ASMInstruction dummy_inst = {.next = NULL}, *inst, *prev_inst = &dummy_inst;
    ASMData dummy_data = {.next = NULL}, *data, *prev_data = &dummy_data;
    const ASMLine *line = state->lines, *origin = NULL;
    size_t offset = 0;
    ssize_t slot = -1;

    for (size_t i = 0; i < HEADER_SIZE; i++)
        overlap_table[state->header.offset + i] = &header_sentinel;

    while (line) {
        if (line->is_label) {
            if ((ei = add_label_to_table(state->symtable, line, offset, slot)))
                goto cleanup;
        }
        else if (IS_LOCAL_DIRECTIVE(line)) {
            if (IS_DIRECTIVE(line, DIR_ORIGIN)) {
                if ((ei = handle_origin_directive(line, &offset, &slot)))
                    goto cleanup;
                origin = line;
            }
            else if (IS_DIRECTIVE(line, DIR_BLOCK)) {
                if ((ei = handle_block_directive(line, &offset, &slot)))
                    goto cleanup;
                origin = line;
            }
            else {
                if ((ei = parse_data(line, &data, offset)))
                    goto cleanup;

                offset += data->loc.length;
                prev_data->next = data;
                prev_data = data;

                if ((ei = check_layout(overlap_table, size, &data->loc, line, origin)))
                    goto cleanup;
            }
        }
        else {
            if ((ei = parse_instruction(line, &inst, offset)))
                goto cleanup;

            offset += inst->loc.length;
            prev_inst->next = inst;
            prev_inst = inst;

            if ((ei = check_layout(overlap_table, size, &inst->loc, line, origin)))
                goto cleanup;
        }
        line = line->next;
    }

    cleanup:
    state->instructions = dummy_inst.next;
    state->data = dummy_data.next;
    free(overlap_table);
    return ei;
}
