/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdlib.h>

#include "assembler.h"
#include "assembler/directives.h"
#include "assembler/errors.h"
#include "assembler/io.h"
#include "assembler/parse_util.h"
#include "assembler/preprocessor.h"
#include "assembler/state.h"
#include "logging.h"
#include "rom.h"
#include "util.h"

#define IS_LABEL(line) (line->data[line->length - 1] == ':')

/* Sentinel values for overlap table */
const ASMLine header_sentinel, bounds_sentinel;

/*
    Add a given line, representing a label, to the symbol table.

    Return NULL on success and an ErrorInfo object on failure (in the case of
    duplicate labels).
*/
static ErrorInfo* add_label_to_table(
    ASMSymbolTable *symtable, const ASMLine *line, size_t offset)
{
    char *symbol = strndup(line->data, line->length - 1);
    if (!symbol)
        OUT_OF_MEMORY()

    const ASMSymbol *current = asm_symtable_find(symtable, symbol);
    if (current) {
        ErrorInfo *ei = error_info_create(line, ET_LAYOUT, ED_LYT_DUPE_LABELS);
        error_info_append(ei, current->line);
        return ei;
    }

    ASMSymbol *label = malloc(sizeof(ASMSymbol));
    if (!label)
        OUT_OF_MEMORY()

    label->offset = offset;
    label->symbol = symbol;
    label->line = line;
    asm_symtable_insert(symtable, label);
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
    const ASMLine *clash = NULL;

    if (loc->offset + loc->length >= size) {
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
    Tokenize ASMLines into ASMInstructions.

    NULL is returned on success and an ErrorInfo object is returned on failure.
    state->instructions, state->data, and state->symtable may or may not be
    modified regardless of success.
*/
static ErrorInfo* tokenize(AssemblerState *state)
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

    for (size_t i = 0; i < HEADER_SIZE; i++)
        overlap_table[state->header.offset + i] = &header_sentinel;

    while (line) {
        if (IS_LABEL(line)) {
            if ((ei = add_label_to_table(state->symtable, line, offset)))
                goto cleanup;
        }
        else if (IS_LOCAL_DIRECTIVE(line)) {
            if (IS_DIRECTIVE(line, DIR_ORIGIN)) {
                if (!DIRECTIVE_HAS_ARG(line, DIR_ORIGIN)) {
                    ei = error_info_create(line, ET_PREPROC, ED_PP_NO_ARG);
                    goto cleanup;
                }

                uint32_t arg;
                if (!dparse_uint32_t(&arg, line, DIR_ORIGIN)) {
                    ei = error_info_create(line, ET_PREPROC, ED_PP_BAD_ARG);
                    goto cleanup;
                }

                offset = arg;
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

/*
    Resolve default placeholder values in assembler state, such as ROM size.

    On success, no new heap objects are allocated. On error, an ErrorInfo
    object is returned.
*/
static ErrorInfo* resolve_defaults(AssemblerState *state)
{
    if (!state->rom_size) {
        state->rom_size = ROM_SIZE_MIN;

        // TODO: use highest instruction too

        if (state->header.rom_size != INVALID_SIZE_CODE) {
            size_t decl_size = size_code_to_bytes(state->header.rom_size);
            if (decl_size > state->rom_size)
                state->rom_size = decl_size;
        }
    }

    if (state->header.rom_size == INVALID_SIZE_CODE)
        state->header.rom_size = size_bytes_to_code(state->rom_size);

    return NULL;
}

/*
    Resolve symbol placeholders in instructions such as jumps and branches.

    On success, no new heap objects are allocated. On error, an ErrorInfo
    object is returned.
*/
static ErrorInfo* resolve_symbols(AssemblerState *state)
{
    // TODO

    (void) state;
    return NULL;
}

/*
    Convert finalized ASMInstructions and ASMData into a binary data block.

    This function should never fail.
*/
static void serialize_binary(AssemblerState *state, uint8_t *binary)
{
    // TODO

    for (size_t i = 0; i < state->rom_size; i++)
        binary[i] = 'X';
}

/*
    Assemble the z80 source code in the source code buffer into binary data.

    If successful, return the size of the assembled binary data and change
    *binary_ptr to point to the assembled ROM data buffer. *binary_ptr must be
    free()'d when finished.

    If an error occurred, return 0 and update *ei_ptr to point to an ErrorInfo
    object which can be shown to the user with error_info_print(). The
    ErrorInfo object must be destroyed with error_info_destroy() when finished.

    In either case, only one of *binary_ptr and *ei_ptr is modified.
*/
size_t assemble(const LineBuffer *source, uint8_t **binary_ptr, ErrorInfo **ei_ptr)
{
    AssemblerState state;
    ErrorInfo *error_info;
    size_t retval = 0;

    state_init(&state);

    if ((error_info = preprocess(&state, source)))
        goto error;

    asm_symtable_init(&state.symtable);

#ifdef DEBUG_MODE
    asm_lines_print(state.lines);
#endif

    if ((error_info = tokenize(&state)))
        goto error;

    if ((error_info = resolve_defaults(&state)))
        goto error;

    if ((error_info = resolve_symbols(&state)))
        goto error;

    uint8_t *binary = malloc(sizeof(uint8_t) * state.rom_size);
    if (!binary)
        OUT_OF_MEMORY()

    serialize_binary(&state, binary);
    *binary_ptr = binary;
    retval = state.rom_size;
    goto cleanup;

    error:
    *ei_ptr = error_info;

    cleanup:
    state_free(&state);
    return retval;
}

/*
    Assemble the z80 source code at the input path into a binary file.

    Return true if the operation was a success and false if it was a failure.
    Errors are printed to STDOUT; if the operation was successful then nothing
    is printed.
*/
bool assemble_file(const char *src_path, const char *dst_path)
{
    LineBuffer *source = read_source_file(src_path, true);
    if (!source)
        return false;

    uint8_t *binary;
    ErrorInfo *error_info;
    size_t size = assemble(source, &binary, &error_info);
    line_buffer_free(source);

    if (!size) {
        error_info_print(error_info, stderr);
        error_info_destroy(error_info);
        return false;
    }

    bool success = write_binary_file(dst_path, binary, size);
    free(binary);
    return success;
}
