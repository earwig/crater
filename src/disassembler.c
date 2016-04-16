/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include "disassembler.h"
#include "disassembler/mnemonics.h"
#include "disassembler/sizes.h"
#include "util.h"

/*
    Format a sequence of bytes of a certain length as a pretty string.

    The result must be freed by the caller.
*/
static char* format_bytestring(const uint8_t *bytes, size_t size)
{
    // TODO: smarter alignment; pad to full len (then remove pad from TRACE())
    if (!size)
        return NULL;

    char *str = cr_malloc(sizeof(char) * (3 * size));
    size_t i;

    for (i = 0; i < size; i++) {
        snprintf(&str[3 * i], 3, "%02X", bytes[i]);
        str[3 * i + 2] = ' ';
    }
    str[3 * size - 1] = '\0';
    return str;
}

/*
    Extract the arguments for the given instruction.

    The return value must be free()d.
*/
static char* decode_argument(const uint8_t *bytes)
{
    // TODO
    (void) bytes;
    return NULL;
}

/*
    Free the given DisasInstr struct.
*/
void disas_instr_free(DisasInstr *instr)
{
    free(instr->bytestr);
    free(instr->line);
    free(instr);
}

/*
    Disassemble a single instruction starting at the given address.

    Return a dynamically allocated structure containing various interesting
    fields. This must be freed by the user with disas_instr_free().
*/
DisasInstr* disassemble_instruction(const uint8_t *bytes)
{
    size_t size = get_instr_size(bytes);
    char *bytestr = format_bytestring(bytes, size);
    char *mnemonic = decode_mnemonic(bytes);
    char *arg = decode_argument(bytes);
    char *line;

    if (arg) {
        line = cr_malloc(strlen(mnemonic) + strlen(arg) + 2);
        sprintf(line, "%s\t%s", mnemonic, arg);
        free(arg);
    } else {
        line = cr_strdup(mnemonic);
    }

    DisasInstr *instr = cr_malloc(sizeof(DisasInstr));
    instr->size = size;
    instr->bytestr = bytestr;
    instr->line = line;
    return instr;
}

/*
    Disassemble the binary file at the input path into z80 source code.

    Return true if the operation was a success and false if it was a failure.
    Errors are printed to STDOUT; if the operation was successful then nothing
    is printed.
*/
bool disassemble_file(const char *src_path, const char *dst_path)
{
    // TODO
    (void) src_path;
    (void) dst_path;
    return true;
}
