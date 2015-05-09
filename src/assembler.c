/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdlib.h>

#include "assembler.h"
#include "assembler/errors.h"
#include "assembler/io.h"
#include "assembler/preprocessor.h"
#include "assembler/state.h"
#include "assembler/tokenizer.h"
#include "logging.h"
#include "rom.h"
#include "util.h"

/*
    Return the smallest ROM size that can contain the given address.

    This uses bit twiddling hacks up to the largest possible ROM size.
*/
static size_t bounding_rom_size(size_t size)
{
    size--;
    size |= size >> 1;
    size |= size >> 2;
    size |= size >> 4;
    size |= size >> 8;
    size |= size >> 16;
    size++;
    return size;
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

        const ASMInstruction *inst = state->instructions;
        while (inst) {
            size_t bound = inst->loc.offset + inst->loc.length;
            if (bound > state->rom_size)
                state->rom_size = bounding_rom_size(bound);
            inst = inst->next;
        }

        const ASMData *data = state->data;
        while (data) {
            size_t bound = data->loc.offset + data->loc.length;
            if (bound > state->rom_size)
                state->rom_size = bounding_rom_size(bound);
            data = data->next;
        }

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
    ErrorInfo *ei;
    ASMInstruction *inst = state->instructions;
    const ASMSymbol *symbol;

    while (inst) {
        if (inst->symbol) {
            symbol = asm_symtable_find(state->symtable, inst->symbol);
            if (!symbol) {
                ei = error_info_create(inst->line, ET_SYMBOL, ED_SYM_NO_LABEL);
                return ei;
            }

            inst->bytes[inst->loc.length - 2] = symbol->offset & 0xFF;
            inst->bytes[inst->loc.length - 1] = symbol->offset >> 8;
            free(inst->symbol);
            inst->symbol = NULL;
        }
        inst = inst->next;
    }
    return NULL;
}

/*
    Write the ROM header to the binary. Header contents are explained in rom.c.
*/
static void write_header(const ASMHeaderInfo *info, uint8_t *binary)
{
    uint8_t *header = binary + info->offset;

    // Bytes 0-7: magic string
    memcpy(header, rom_header_magic, HEADER_MAGIC_LEN);
    header += HEADER_MAGIC_LEN;

    // Bytes 8, 9: unused
    *(header++) = 0x00;
    *(header++) = 0x00;

    // Bytes A, B: checksum
    if (info->checksum) {
        uint16_t checksum = compute_checksum(binary, 0, info->rom_size);
        *header = checksum & 0xFF;
        *(header + 1) = checksum >> 8;
    } else {
        *header = *(header + 1) = 0x00;
    }
    header += 2;

    // Bytes C, D: product code (least significant two bytes)
    *header = bcd_encode(info->product_code % 100);
    *(header + 1) = bcd_encode((info->product_code / 100) % 100);
    header += 2;

    // Byte E: product code (most significant nibble), version
    *header = (info->product_code / 10000) << 4 | (info->version & 0x0F);
    header++;

    // Byte F: region code, ROM size
    *header = (info->region << 4) | (info->rom_size & 0x0F);
}

/*
    Convert finalized ASMInstructions and ASMData into a binary data block.

    This function should never fail.
*/
static void serialize_binary(const AssemblerState *state, uint8_t *binary)
{
    memset(binary, 0xFF, state->rom_size);

    const ASMInstruction *inst = state->instructions;
    while (inst) {
        memcpy(binary + inst->loc.offset, inst->bytes, inst->loc.length);
        inst = inst->next;
    }

    const ASMData *data = state->data;
    while (data) {
        memcpy(binary + data->loc.offset, data->bytes, data->loc.length);
        data = data->next;
    }

    write_header(&state->header, binary);
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

    uint8_t *binary = cr_malloc(sizeof(uint8_t) * state.rom_size);
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
    DEBUG("Assembling: %s -> %s", src_path, dst_path)
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
