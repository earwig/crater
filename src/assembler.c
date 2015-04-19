/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdlib.h>

#include "assembler.h"
#include "assembler/errors.h"
#include "assembler/io.h"
#include "assembler/preprocessor.h"
#include "assembler/state.h"
#include "logging.h"
#include "rom.h"
#include "util.h"

/*
    Tokenize ASMLines into ASMInstructions.

    On success, state->instructions is modified and NULL is returned. On error,
    an ErrorInfo object is returned and state->instructions is not modified.
    state->symtable may or may not be modified regardless of success.
*/
static ErrorInfo* tokenize(AssemblerState *state)
{
    // TODO

    // verify no instructions clash with header offset
    // if rom size is set, verify nothing overflows
    // otherwise, check nothing overflows max rom size (1 MB)

    (void) state;

#ifdef DEBUG_MODE
    DEBUG("Dumping ASMLines:")
    const ASMLine *temp = state->lines;
    while (temp) {
        DEBUG("- %-40.*s [%s:%02zu]", (int) temp->length, temp->data,
              temp->filename, temp->original->lineno)
        temp = temp->next;
    }
#endif

    return NULL;
}

/*
    Resolve default placeholder values in assembler state, such as ROM size.

    On success, no new heap objects are allocated. On error, an ErrorInfo
    object is returned.
*/
static ErrorInfo* resolve_defaults(AssemblerState *state)
{
    if (!state->rom_size) {
        state->rom_size = 32 << 10;

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
    Convert finalized ASMInstructions into a binary data block.

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
    asm_lines_free(state.lines);
    asm_includes_free(state.includes);
    asm_instructions_free(state.instructions);
    asm_symtable_free(state.symtable);
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
