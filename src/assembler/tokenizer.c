/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdlib.h>
#include <string.h>

#include "tokenizer.h"
#include "directives.h"
#include "instructions.h"
#include "inst_args.h"
#include "parse_util.h"
#include "../mmu.h"
#include "../rom.h"
#include "../util.h"

/* Internal structs */

typedef struct {
    size_t size;
    const ASMLine **overlap_table;
    const ASMLine **overlap_origins;
    const ASMLine *origin;
    uint8_t bank;
    bool cross_blocks;
} ASMLayoutInfo;

typedef struct {
    int8_t slots[MMU_NUM_ROM_BANKS];
    const ASMLine *lines[MMU_NUM_ROM_BANKS];
} ASMSlotInfo;

/* Sentinel values for overlap table */

const ASMLine header_sentinel, bounds_sentinel;

/* Typedef for parse_util data parser functions */

typedef bool (*parser_func)(uint8_t**, size_t*, const char*, ssize_t);

/*
    Return the address of a given ROM offset when mapped into the given slot.
*/
static inline uint16_t map_into_slot(size_t offset, int8_t slot)
{
    return (slot * MMU_ROM_BANK_SIZE) + (offset & (MMU_ROM_BANK_SIZE - 1));
}

/*
    Return the default slot associated with a given memory bank.
*/
static inline int8_t default_bank_slot(uint8_t bank)
{
    return bank > 2 ? 2 : bank;
}

/*
    Initialize an ASMLayoutInfo object.
*/
static void init_layout_info(ASMLayoutInfo *li, AssemblerState *state)
{
    li->size = state->rom_size ? state->rom_size : ROM_SIZE_MAX;
    li->origin = NULL;
    li->bank = 0;
    li->cross_blocks = state->cross_blocks;
    li->overlap_table = cr_calloc(li->size, sizeof(const ASMLine*));
    li->overlap_origins = cr_calloc(li->size, sizeof(const ASMLine*));

    for (size_t i = 0; i < HEADER_SIZE; i++)
        li->overlap_table[state->header.offset + i] = &header_sentinel;
}

/*
    Free the resources allocated by an ASMLayoutInfo object.
*/
static void free_layout_info(ASMLayoutInfo *li)
{
    free(li->overlap_table);
    free(li->overlap_origins);
}

/*
    Add a given line, representing a label, to the symbol table.

    Return NULL on success and an ErrorInfo object on failure (e.g. in the case
    of duplicate labels, or labels sharing names with registers/conditions).
*/
static ErrorInfo* add_label_to_table(
    ASMSymbolTable *symtable, const ASMLine *line, size_t offset, int8_t slot)
{
    if (line->length - 1 >= MAX_SYMBOL_SIZE)
        return error_info_create(line, ET_SYMBOL, ED_SYM_TOO_LONG);

    ASMArgParseInfo info = {.arg = line->data, .size = line->length - 1};
    ASMArgRegister reg;
    if (argparse_register(&reg, info))
        return error_info_create(line, ET_SYMBOL, ED_SYM_IS_REGISTER);

    ASMArgCondition cond;
    if (argparse_condition(&cond, info))
        return error_info_create(line, ET_SYMBOL, ED_SYM_IS_CONDITION);

    char *symbol = cr_strndup(line->data, line->length - 1);
    const ASMSymbol *current = asm_symtable_find(symtable, symbol);
    if (current) {
        ErrorInfo *ei = error_info_create(line, ET_SYMBOL, ED_SYM_DUPE_LABELS);
        error_info_append(ei, current->line);
        free(symbol);
        return ei;
    }

    ASMSymbol *label = cr_malloc(sizeof(ASMSymbol));
    label->offset = map_into_slot(offset,
        (slot >= 0) ? slot : default_bank_slot(offset / MMU_ROM_BANK_SIZE));
    label->symbol = symbol;
    label->line = line;
    asm_symtable_insert(symtable, label);
    return NULL;
}

/*
    Handle a define directive by adding an entry to the define table.

    Return NULL on success and an ErrorInfo object on failure.
*/
static ErrorInfo* handle_define_directive(
    const ASMLine *line, ASMDefineTable *deftab)
{
    if (!DIRECTIVE_HAS_ARG(line, DIR_DEFINE))
        return error_info_create(line, ET_PREPROC, ED_PP_NO_ARG);

    size_t start = DIRECTIVE_OFFSET(line, DIR_DEFINE) + 1, i;
    for (i = start; i < line->length; i++) {
        if (!is_valid_symbol_char(line->data[i], i == start)) {
            if (line->data[i] == ' ' && i > start) {
                i++;
                break;
            }
            return error_info_create(line, ET_PREPROC, ED_PP_BAD_ARG);
        }
    }

    if (i >= line->length)  // Missing value for define
        return error_info_create(line, ET_PREPROC, ED_PP_NO_ARG);

    const char *key = line->data + start;
    size_t keylen = i - start - 1;

    const ASMDefine *current = asm_deftable_find(deftab, key, keylen);
    if (current) {
        ErrorInfo *ei = error_info_create(line, ET_PREPROC, ED_PP_DUPLICATE);
        error_info_append(ei, current->line);
        return ei;
    }

    ASMArgImmediate imm;
    ASMArgParseInfo info = {
        .arg = line->data + i, .size = line->length - i, .deftable = deftab};
    if (!argparse_immediate(&imm, info))
        return error_info_create(line, ET_PREPROC, ED_PP_BAD_ARG);

    ASMDefine *define = cr_malloc(sizeof(ASMDefine));
    define->name = cr_strndup(key, keylen);
    define->value = imm;
    define->line = line;
    asm_deftable_insert(deftab, define);
    return NULL;
}

/*
    Handle an undefine directive by remove an entry in the define table.

    Return NULL on success and an ErrorInfo object on failure.
*/
static ErrorInfo* handle_undef_directive(
    const ASMLine *line, ASMDefineTable *deftab)
{
    if (!DIRECTIVE_HAS_ARG(line, DIR_UNDEF))
        return error_info_create(line, ET_PREPROC, ED_PP_NO_ARG);

    size_t offset = DIRECTIVE_OFFSET(line, DIR_UNDEF) + 1;
    const char *arg = line->data + offset;
    size_t size = line->length - offset, i;

    for (i = 0; i < size; i++) {
        if (!is_valid_symbol_char(arg[i], i == 0))
            return error_info_create(line, ET_PREPROC, ED_PP_BAD_ARG);
    }

    asm_deftable_remove(deftab, arg, size);
    return NULL;
}

/*
    Handle an origin directive by updating the offset.

    Return NULL on success and an ErrorInfo object on failure.
*/
static ErrorInfo* handle_origin_directive(const ASMLine *line, size_t *offset)
{
    if (!DIRECTIVE_HAS_ARG(line, DIR_ORIGIN))
        return error_info_create(line, ET_PREPROC, ED_PP_NO_ARG);

    uint32_t arg;
    if (!dparse_uint32_t(&arg, line, DIR_ORIGIN))
        return error_info_create(line, ET_PREPROC, ED_PP_BAD_ARG);

    if (arg >= ROM_SIZE_MAX)
        return error_info_create(line, ET_PREPROC, ED_PP_ARG_RANGE);

    *offset = arg;
    return NULL;
}

/*
    Handle a block directive by updating the offset and slot.

    Return NULL on success and an ErrorInfo object on failure.
*/
static ErrorInfo* handle_block_directive(
    const ASMLine *line, size_t *offset, ASMSlotInfo *si)
{
    if (!DIRECTIVE_HAS_ARG(line, DIR_BLOCK))
        return error_info_create(line, ET_PREPROC, ED_PP_NO_ARG);

    uint8_t *args, bank, slot;
    size_t dir_offset = DIRECTIVE_OFFSET(line, DIR_BLOCK) + 1, nargs;

    if (!parse_bytes(&args, &nargs, line->data + dir_offset,
                     line->length - dir_offset))
        return error_info_create(line, ET_PREPROC, ED_PP_BAD_ARG);
    if (nargs < 1 || nargs > 2)
        return free(args), error_info_create(line, ET_PREPROC, ED_PP_BAD_ARG);

    bank = args[0];
    slot = nargs == 2 ? args[1] : default_bank_slot(bank);
    free(args);

    if (bank >= MMU_NUM_ROM_BANKS || slot >= MMU_NUM_SLOTS)
        return error_info_create(line, ET_PREPROC, ED_PP_ARG_RANGE);
    if (bank == 0 && slot != 0)
        return error_info_create(line, ET_LAYOUT, ED_LYT_BLOCK0);
    if (si->slots[bank] >= 0 && si->slots[bank] != slot) {
        ErrorInfo *ei = error_info_create(line, ET_LAYOUT, ED_LYT_SLOTS);
        error_info_append(ei, si->lines[bank]);
        return ei;
    }

    *offset = bank * MMU_ROM_BANK_SIZE;
    si->slots[bank] = slot;
    if (!si->lines[bank])
        si->lines[bank] = line;
    return NULL;
}

/*
    Parse a .space directive, which fills a region with a single byte.
*/
static bool parse_space(
    uint8_t **result, size_t *length, const char *arg, ssize_t size)
{
    uint8_t *bytes;
    size_t nbytes;
    if (!parse_bytes(&bytes, &nbytes, arg, size))
        return false;

    if (nbytes < 1 || nbytes > 2) {
        free(bytes);
        return false;
    }

    *length = bytes[0];
    *result = cr_malloc(sizeof(uint8_t) * (*length));
    memset(*result, nbytes == 2 ? bytes[1] : 0, *length);
    free(bytes);
    return true;
}

/*
    Parse data encoded in a line into an ASMData object.

    On success, return NULL and store the instruction in *data_ptr. On failure,
    return an ErrorInfo object; *data_ptr is not modified.
*/
static ErrorInfo* parse_data(
    const ASMLine *line, ASMData **data_ptr, size_t offset)
{
    const char *directive;
    parser_func parser = (parser_func) parse_string;

    if (IS_DIRECTIVE(line, DIR_BYTE)) {
        directive = DIR_BYTE;
        parser = parse_bytes;
    } else if (IS_DIRECTIVE(line, DIR_SPACE)) {
        directive = DIR_SPACE;
        parser = parse_space;
    } else if (IS_DIRECTIVE(line, DIR_ASCII)) {
        directive = DIR_ASCII;
    } else if (IS_DIRECTIVE(line, DIR_ASCIZ)) {
        directive = DIR_ASCIZ;
    } else if (IS_DIRECTIVE(line, DIR_ASCIIZ)) {
        directive = DIR_ASCIIZ;
    } else {
        return error_info_create(line, ET_PREPROC, ED_PP_UNKNOWN);
    }

    if (!DIRECTIVE_HAS_ARG(line, directive))
        return error_info_create(line, ET_PREPROC, ED_PP_NO_ARG);

    size_t dir_offset = DIRECTIVE_OFFSET(line, directive) + 1;
    const char *arg = line->data + dir_offset;
    size_t arglen = line->length - dir_offset;

    ASMData *data = cr_malloc(sizeof(ASMData));
    data->loc.offset = offset;
    data->next = NULL;

    if (!parser(&data->bytes, &data->loc.length, arg, arglen)) {
        free(data);
        return error_info_create(line, ET_PREPROC, ED_PP_BAD_ARG);
    }

    *data_ptr = data;
    return NULL;
}

/*
    Parse an instruction encoded in a line into an ASMInstruction object.

    On success, return NULL and store the instruction in *inst_ptr. On failure,
    return an ErrorInfo object; *inst_ptr is not modified.
*/
static ErrorInfo* parse_instruction(
    const ASMLine *line, ASMInstruction **inst_ptr, size_t offset,
    ASMDefineTable *deftab)
{
    char mnemonic[MAX_MNEMONIC_SIZE] = {0};
    size_t i = 0;
    while (i < line->length) {
        char c = line->data[i];
        if (c == ' ')
            break;
        if (i >= MAX_MNEMONIC_SIZE)
            return error_info_create(line, ET_PARSER, ED_PS_OP_TOO_LONG);
        if ((c < 'a' || c > 'z') && (c < '0' || c > '9'))
            return error_info_create(line, ET_PARSER, ED_PS_OP_INVALID);
        mnemonic[i++] = c;
    }

    if (i < MIN_MNEMONIC_SIZE)
        return error_info_create(line, ET_PARSER, ED_PS_OP_TOO_SHORT);

    if (i + 1 < line->length)
        i++;  // Advance past space

    uint8_t *bytes;
    size_t arglen = line->length - i, length;
    char *argstart = arglen > 0 ? line->data + i : NULL, *symbol = NULL;

    ASMInstParser parser = get_inst_parser(mnemonic);
    if (!parser)
        return error_info_create(line, ET_PARSER, ED_PS_OP_UNKNOWN);

    ASMArgParseInfo ai = {.arg = argstart, .size = arglen, .deftable = deftab};
    ASMErrorDesc edesc = parser(&bytes, &length, &symbol, ai);
    if (edesc != ED_NONE)
        return error_info_create(line, ET_PARSER, edesc);

    ASMInstruction *inst = cr_malloc(sizeof(ASMInstruction));
    inst->loc.offset = offset;
    inst->loc.length = length;
    inst->bytes = bytes;
    inst->symbol = symbol;
    inst->line = line;
    inst->next = NULL;

    *inst_ptr = inst;
    return NULL;
}

/*
    Check if the given object location is legal.

    Checks include ROM size bounding, overlapping with existing objects, and
    block-crossing assuming the .cross_blocks directive has not been specified.

    On success, return NULL and add the location to the overlap table.
    On failure, return an ErrorInfo object.
*/
static ErrorInfo* check_layout(
    ASMLayoutInfo *li, const ASMLocation *loc, const ASMLine *line)
{
    const ASMLine *clash = NULL, *clash_origin;
    if (loc->offset + loc->length > li->size) {
        clash = &bounds_sentinel;
    } else {
        for (size_t i = 0; i < loc->length; i++) {
            if (li->overlap_table[loc->offset + i]) {
                clash = li->overlap_table[loc->offset + i];
                clash_origin = li->overlap_origins[loc->offset + i];
                break;
            }
        }
    }

    if (clash) {
        ErrorInfo *ei = error_info_create(line, ET_LAYOUT,
            (clash == &header_sentinel) ? ED_LYT_OVERLAP_HEAD :
            (clash == &bounds_sentinel) ? ED_LYT_BOUNDS : ED_LYT_OVERLAP);

        if (li->origin)
            error_info_append(ei, li->origin);
        if (clash != &header_sentinel && clash != &bounds_sentinel) {
            error_info_append(ei, clash);
            if (clash_origin)
                error_info_append(ei, clash_origin);
        }
        return ei;
    }

    uint8_t bank = (loc->offset + loc->length - 1) / MMU_ROM_BANK_SIZE;
    if (bank != li->bank && !li->cross_blocks) {
        ErrorInfo *ei = error_info_create(line, ET_LAYOUT, ED_LYT_BLOCK_CROSS);
        if (li->origin)
            error_info_append(ei, li->origin);
        return ei;
    }

    for (size_t i = 0; i < loc->length; i++) {
        li->overlap_table[loc->offset + i] = line;
        li->overlap_origins[loc->offset + i] = li->origin;
    }
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
    ErrorInfo *ei = NULL;
    ASMLayoutInfo li;
    ASMSlotInfo si = {.lines = {0}};
    ASMDefineTable *deftab = asm_deftable_new();
    ASMInstruction dummy_inst = {.next = NULL}, *inst, *prev_inst = &dummy_inst;
    ASMData dummy_data = {.next = NULL}, *data, *prev_data = &dummy_data;
    const ASMLine *line = state->lines;
    size_t offset = 0;

    init_layout_info(&li, state);
    memset(si.slots, -1, MMU_NUM_ROM_BANKS);

    while (line) {
        if (line->is_label) {
            if (offset >= li.size) {
                ei = error_info_create(line, ET_LAYOUT, ED_LYT_BOUNDS);
                goto cleanup;
            }
            int8_t slot = si.slots[offset / MMU_NUM_ROM_BANKS];
            if ((ei = add_label_to_table(state->symtable, line, offset, slot)))
                goto cleanup;
        }
        else if (IS_LOCAL_DIRECTIVE(line)) {
            if (IS_DIRECTIVE(line, DIR_DEFINE)) {
                if ((ei = handle_define_directive(line, deftab)))
                    goto cleanup;
            }
            else if (IS_DIRECTIVE(line, DIR_UNDEF)) {
                if ((ei = handle_undef_directive(line, deftab)))
                    goto cleanup;
            }
            else if (IS_DIRECTIVE(line, DIR_ORIGIN)) {
                if ((ei = handle_origin_directive(line, &offset)))
                    goto cleanup;

                li.origin = line;
                li.bank = offset / MMU_ROM_BANK_SIZE;
            }
            else if (IS_DIRECTIVE(line, DIR_BLOCK)) {
                if ((ei = handle_block_directive(line, &offset, &si)))
                    goto cleanup;

                li.origin = line;
                li.bank = offset / MMU_ROM_BANK_SIZE;
            }
            else {
                if ((ei = parse_data(line, &data, offset)))
                    goto cleanup;

                offset += data->loc.length;
                prev_data->next = data;
                prev_data = data;

                if ((ei = check_layout(&li, &data->loc, line)))
                    goto cleanup;
            }
        }
        else {
            if ((ei = parse_instruction(line, &inst, offset, deftab)))
                goto cleanup;

            offset += inst->loc.length;
            prev_inst->next = inst;
            prev_inst = inst;

            if ((ei = check_layout(&li, &inst->loc, line)))
                goto cleanup;
        }
        line = line->next;
    }

    cleanup:
    state->instructions = dummy_inst.next;
    state->data = dummy_data.next;
    free_layout_info(&li);
    asm_deftable_free(deftab);
    return ei;
}
