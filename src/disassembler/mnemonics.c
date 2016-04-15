/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include "mnemonics.h"

static char* instr_mnemonics[256] = {
    /* 00 */ "nop",  "ld",   "ld",   "inc",  "inc",  "dec",  "ld",   "rlca",
    /* 08 */ "ex",   "add",  "ld",   "dec",  "inc",  "dec",  "ld",   "rrca",
    /* 10 */ "djnz", "ld",   "ld",   "inc",  "inc",  "dec",  "ld",   "rla",
    /* 18 */ "jr",   "add",  "ld",   "dec",  "inc",  "dec",  "ld",   "rra",
    /* 20 */ "jr",   "ld",   "ld",   "inc",  "inc",  "dec",  "ld",   "daa",
    /* 28 */ "jr",   "add",  "ld",   "dec",  "inc",  "dec",  "ld",   "cpl",
    /* 30 */ "jr",   "ld",   "ld",   "inc",  "inc",  "dec",  "ld",   "scf",
    /* 38 */ "jr",   "add",  "ld",   "dec",  "inc",  "dec",  "ld",   "ccf",
    /* 40 */ "ld",   "ld",   "ld",   "ld",   "ld",   "ld",   "ld",   "ld",
    /* 48 */ "ld",   "ld",   "ld",   "ld",   "ld",   "ld",   "ld",   "ld",
    /* 50 */ "ld",   "ld",   "ld",   "ld",   "ld",   "ld",   "ld",   "ld",
    /* 58 */ "ld",   "ld",   "ld",   "ld",   "ld",   "ld",   "ld",   "ld",
    /* 60 */ "ld",   "ld",   "ld",   "ld",   "ld",   "ld",   "ld",   "ld",
    /* 68 */ "ld",   "ld",   "ld",   "ld",   "ld",   "ld",   "ld",   "ld",
    /* 70 */ "ld",   "ld",   "ld",   "ld",   "ld",   "ld",   "halt", "ld",
    /* 78 */ "ld",   "ld",   "ld",   "ld",   "ld",   "ld",   "ld",   "ld",
    /* 80 */ "add",  "add",  "add",  "add",  "add",  "add",  "add",  "add",
    /* 88 */ "adc",  "adc",  "adc",  "adc",  "adc",  "adc",  "adc",  "adc",
    /* 90 */ "sub",  "sub",  "sub",  "sub",  "sub",  "sub",  "sub",  "sub",
    /* 98 */ "sbc",  "sbc",  "sbc",  "sbc",  "sbc",  "sbc",  "sbc",  "sbc",
    /* A0 */ "and",  "and",  "and",  "and",  "and",  "and",  "and",  "and",
    /* A8 */ "xor",  "xor",  "xor",  "xor",  "xor",  "xor",  "xor",  "xor",
    /* B0 */ "or",   "or",   "or",   "or",   "or",   "or",   "or",   "or",
    /* B8 */ "cp",   "cp",   "cp",   "cp",   "cp",   "cp",   "cp",   "cp",
    /* C0 */ "ret",  "pop",  "jp",   "jp",   "call", "push", "add",  "rst",
    /* C8 */ "ret",  "ret",  "jp",   "",     "call", "call", "adc",  "rst",
    /* D0 */ "ret",  "pop",  "jp",   "out",  "call", "push", "sub",  "rst",
    /* D8 */ "ret",  "exx",  "jp",   "in",   "call", "",     "sbc",  "rst",
    /* E0 */ "ret",  "pop",  "jp",   "ex",   "call", "push", "and",  "rst",
    /* E8 */ "ret",  "jp",   "jp",   "ex",   "call", "",     "xor",  "rst",
    /* F0 */ "ret",  "pop",  "jp",   "di",   "call", "push", "or",   "rst",
    /* F8 */ "ret",  "ld",   "jp",   "ei",   "call", "",     "cp",   "rst"
};

/*
    Extract the assembly mnemonic for the given opcode.

    The return value is a string literal and should not be freed.
*/
char* decode_mnemonic(const uint8_t *bytes)
{
    return instr_mnemonics[bytes[0]];  // TODO: extended...
}
