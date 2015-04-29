/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include "instructions.h"
#include "../logging.h"

/*
    TEMP SYNTAX NOTES:
    - http://clrhome.org/table/
    - http://www.z80.info/z80undoc.htm
    - http://www.z80.info/z80code.txt
    - http://www.z80.info/z80href.txt

    instruction := mnemonic [arg[, arg[, arg]]]
    mnemonic    := [a-z0-9]{2-4}
    arg         := register | immediate | label | indirect | indexed | condition | page0

    register  := A | B | C | D | E | AF | BC | DE | HL | F | I | IX | IY | PC | R | SP
    immediate := 8-bit integer | 16-bit integer
    label     := string
    indirect  := \( (register | immediate) \)
    indexed   := \( (IX | IY) + immediate \)
    condition := NZ | N | NC | C | PO | PE | P | M
    page0     := $0 | $8 | $10 | $18 | $20 | $28 | $30 | $38
*/

/* Helper macros for get_inst_parser() */

#define JOIN_(a, b, c, d) ((uint32_t) ((a << 24) + (b << 16) + (c << 8) + d))

#define DISPATCH_(s, z) (                                                     \
    z == 2 ? JOIN_(s[0], s[1], 0x00, 0x00) :                                  \
    z == 3 ? JOIN_(s[0], s[1], s[2], 0x00) :                                  \
             JOIN_(s[0], s[1], s[2], s[3]))                                   \

#define MAKE_CMP_(s) DISPATCH_(s, (sizeof(s) / sizeof(char) - 1))

#define HANDLE(m) if (key == MAKE_CMP_(#m)) return parse_inst_##m;

/* Instruction parser functions */

#define INST_FUNC(mnemonic)                                                   \
static ASMErrorDesc parse_inst_##mnemonic(                                    \
    uint8_t **bytes, size_t *length, char **symbol, const char *arg, size_t size)

INST_FUNC(nop)
{
    DEBUG("dispatched to -> NOP")
    return ED_PS_TOO_FEW_ARGS;
}

INST_FUNC(inc)
{
    DEBUG("dispatched to -> INC")
    return ED_PS_TOO_FEW_ARGS;
}

INST_FUNC(add)
{
    DEBUG("dispatched to -> ADD")
    return ED_PS_TOO_FEW_ARGS;
}

INST_FUNC(adc)
{
    DEBUG("dispatched to -> ADC")
    return ED_PS_TOO_FEW_ARGS;
}

/*
    Return the relevant ASMInstParser function for a given mnemonic.

    NULL is returned if the mnemonic is not known.
*/
ASMInstParser get_inst_parser(char mstr[MAX_MNEMONIC_SIZE])
{
    // Exploit the fact that we can store the entire mnemonic string as a
    // single 32-bit value to do fast lookups:
    uint32_t key = (mstr[0] << 24) + (mstr[1] << 16) + (mstr[2] << 8) + mstr[3];

    DEBUG("get_inst_parser(): -->%.*s<-- 0x%08X", MAX_MNEMONIC_SIZE, mstr, key)

    HANDLE(nop)
    HANDLE(inc)
    HANDLE(add)
    HANDLE(adc)

    return NULL;
}
