/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

/*
    This file contains code to implement the Z80 instruction set. Since there
    are a lot of functions, it is kept separate from the main z80.c file. It is
    included in the middle of z80.c and should not be compiled separately.
*/

/*
    0x00: NOP
*/
static uint8_t z80_inst_nop(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    z80->regfile.pc++;
    return 4;
}

/*
    Unimplemented opcode handler.
*/
static uint8_t z80_inst_ERR(Z80 *z80, uint8_t opcode)
{
    z80->except = true;
    z80->exc_code = Z80_EXC_UNIMPLEMENTED_OPCODE;
    z80->exc_data = opcode;
    return 2;
}

static uint8_t (*instruction_lookup_table[256])(Z80*, uint8_t) = {
    z80_inst_nop,
    z80_inst_ERR
};
