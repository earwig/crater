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

/*
    ...
*/
ASMInstParser get_inst_parser(char mnemonic[MAX_MNEMONIC_SIZE + 1])
{
    // TODO
    DEBUG("get_inst_parser(): -->%s<--", mnemonic)

    return NULL;
}
