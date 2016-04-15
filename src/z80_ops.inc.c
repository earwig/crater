/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

/*
    This file contains code to implement the Z80 instruction set. Since there
    are a lot of functions, it is kept separate from the main z80.c file. It is
    included in the middle of z80.c and should not be compiled separately.

    Most of this information can be found in the Z80 User Manual, Revision 06.

    Undocumented opcodes, flags, and some additional details come from:
    - http://clrhome.org/table/
    - http://www.z80.info/z80sflag.htm
*/

/*
    Unimplemented opcode handler.
*/
static uint8_t z80_inst_unimplemented(Z80 *z80, uint8_t opcode)
{
    z80->except = true;
    z80->exc_code = Z80_EXC_UNIMPLEMENTED_OPCODE;
    z80->exc_data = opcode;
    return 2;
}

// LD r, r'

/*
    LD r, n (0x06, 0x0E, 0x16, 0x1E, 0x26, 0x2E, 0x3E):
    Load the immediate value into the 8-bit register.
*/
static uint8_t z80_inst_ld_r_n(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg;

    switch(opcode) {
        case 0x3E: reg = &z80->regfile.a; break;
        case 0x06: reg = &z80->regfile.b; break;
        case 0x0E: reg = &z80->regfile.c; break;
        case 0x16: reg = &z80->regfile.d; break;
        case 0x1E: reg = &z80->regfile.e; break;
        case 0x26: reg = &z80->regfile.h; break;
        case 0x2E: reg = &z80->regfile.l; break;
    }

    *reg = mmu_read_byte(z80->mmu, ++z80->regfile.pc);
    z80->regfile.pc++;
    return 7;
}

// LD r, (HL)

// LD r, (IX+d)

// LD r, (IY+d)

// LD (HL), r

// LD (IX+d), r

// LD (IY+d), r

// LD (HL), n

// LD (IX+d), n

// LD (IY+d), n

// LD A, (BC)

// LD A, (DE)

// LD A, (nn)

// LD (BC), A

// LD (DE), A

// LD (nn), A

// LD A, I

// LD A, R

// LD I,A

// LD R, A

/*
    LD dd, nn (0x01, 0x11, 0x21, 0x31):
    Load the two-byte immediate into the 16-bit register.
*/
static uint8_t z80_inst_ld_dd_nn(Z80 *z80, uint8_t opcode)
{
    uint8_t pair;

    switch(opcode) {
        case 0x01: pair = REG_BC; break;
        case 0x11: pair = REG_DE; break;
        case 0x21: pair = REG_HL; break;
        case 0x31: pair = REG_SP; break;
    }

    set_pair(z80, pair, mmu_read_double(z80->mmu, ++z80->regfile.pc));
    z80->regfile.pc += 2;
    return 6;
}

// LD IX, nn

// LD IY, nn

// LD HL, (nn)

// LD dd, (nn)

// LD IX, (nn)

// LD IY, (nn)

// LD (nn), HL

// LD (nn), dd

// LD (nn), IX

// LD (nn), IY

// LD SP, HL

// LD SP, IX

// LD SP, IY

// PUSH qq

// PUSH IX

// PUSH IY

// POP qq

// POP IX

// POP IY

// EX DE, HL

// EX AF, AF′

// EXX

// EX (SP), HL

// EX (SP), IX

// EX (SP), IY

// LDI

// LDIR

// LDD

// LDDR

// CPI

// CPIR

// CPD

// CPDR

// ADD A, r

// ADD A, n

// ADD A, (HL)

// ADD A, (IX + d)

// ADD A, (IY + d)

// ADC A, s

// SUB s

// SBC A, s

// AND s

// OR s

// XOR s

// CP s

/*
    INC r (0x04, 0x0C, 0x14, 0x1C, 0x24, 0x2C, 0x3C):
    Increment the 8-bit register encoded in the opcode.
*/
static uint8_t z80_inst_inc_r(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg;

    switch(opcode) {
        case 0x3C: reg = &z80->regfile.a; break;
        case 0x04: reg = &z80->regfile.b; break;
        case 0x0C: reg = &z80->regfile.c; break;
        case 0x14: reg = &z80->regfile.d; break;
        case 0x1C: reg = &z80->regfile.e; break;
        case 0x24: reg = &z80->regfile.h; break;
        case 0x2C: reg = &z80->regfile.l; break;
    }

    bool halfcarry = !!(((*reg & 0x0F) + 1) & 0x10);
    (*reg)++;
    update_flags(z80, 0, 0, *reg == 0x80, !!(*reg & 0x08), halfcarry,
                 !!(*reg & 0x20), *reg == 0, !!(*reg & 0x80), 0xFE);

    z80->regfile.pc++;
    return 4;
}

// INC (HL)

// INC (IX+d)

// INC (IY+d)

// DEC m

// DAA

// CPL

// NEG

// CCF

// SCF

/*
    NOP (0x00):
    No operation is performed.
*/
static uint8_t z80_inst_nop(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    z80->regfile.pc++;
    return 4;
}

// HALT

// DI

// EI

// IM 0

// IM 1

// IM 2

// ADD HL, ss

// ADC HL, ss

// SBC HL, ss

// ADD IX, pp

// ADD IY, rr

/*
    INC ss (0x03, 0x13, 0x23, 0x33):
    Increment the 16-bit register encoded in the opcode.
*/
static uint8_t z80_inst_inc_ss(Z80 *z80, uint8_t opcode)
{
    uint8_t pair;

    switch(opcode) {
        case 0x03: pair = REG_BC; break;
        case 0x13: pair = REG_DE; break;
        case 0x23: pair = REG_HL; break;
        case 0x33: pair = REG_SP; break;
    }

    set_pair(z80, pair, get_pair(z80, pair) + 1);
    z80->regfile.pc++;
    return 6;
}

// INC IX

// INC IY

// DEC ss

// DEC IX

// DEC IY

// RLCA

// RLA

// RRCA

// RRA

// RLC r

// RLC (HL)

// RLC (IX+d)

// RLC (IY+d)

// RL m

// RRC m

// RR m

// SLA m

// SRA m

// SRL m

// RLD

// RRD

// BIT b, r

// BIT b, (HL)

// BIT b, (IX+d)

// BIT b, (IY+d)

// SET b, r

// SET b, (HL)

// SET b, (IX+d)

// SET b, (IY+d)

// RES b, m

// JP nn

// JP cc, nn

// JR e

// JR C, e

// JR NC, e

// JR Z, e

// JR NZ, e

// JP (HL)

// JP (IX)

// JP (IY)

// DJNZ, e

// CALL nn

// CALL cc, nn

// RET

// RET cc

// RETI

// RETN

// RST p

// IN A, (n)

// IN r (C)

// INI

// INIR

// IND

// INDR

// OUT (n), A

// OUT (C), r

// OUTI

// OTIR

// OUTD

// OTDR

static uint8_t (*instruction_lookup_table[256])(Z80*, uint8_t) = {
    [0x00] = z80_inst_nop,
    [0x01] = z80_inst_ld_dd_nn,
    [0x02] = z80_inst_unimplemented,  // TODO
    [0x03] = z80_inst_inc_ss,
    [0x04] = z80_inst_inc_r,
    [0x05] = z80_inst_unimplemented,  // TODO
    [0x06] = z80_inst_ld_r_n,
    [0x07] = z80_inst_unimplemented,  // TODO
    [0x08] = z80_inst_unimplemented,  // TODO
    [0x09] = z80_inst_unimplemented,  // TODO
    [0x0A] = z80_inst_unimplemented,  // TODO
    [0x0B] = z80_inst_unimplemented,  // TODO
    [0x0C] = z80_inst_inc_r,
    [0x0D] = z80_inst_unimplemented,  // TODO
    [0x0E] = z80_inst_ld_r_n,
    [0x0F] = z80_inst_unimplemented,  // TODO
    [0x10] = z80_inst_unimplemented,  // TODO
    [0x11] = z80_inst_ld_dd_nn,
    [0x12] = z80_inst_unimplemented,  // TODO
    [0x13] = z80_inst_inc_ss,
    [0x14] = z80_inst_inc_r,
    [0x15] = z80_inst_unimplemented,  // TODO
    [0x16] = z80_inst_ld_r_n,
    [0x17] = z80_inst_unimplemented,  // TODO
    [0x18] = z80_inst_unimplemented,  // TODO
    [0x19] = z80_inst_unimplemented,  // TODO
    [0x1A] = z80_inst_unimplemented,  // TODO
    [0x1B] = z80_inst_unimplemented,  // TODO
    [0x1C] = z80_inst_inc_r,
    [0x1D] = z80_inst_unimplemented,  // TODO
    [0x1E] = z80_inst_ld_r_n,
    [0x1F] = z80_inst_unimplemented,  // TODO
    [0x20] = z80_inst_unimplemented,  // TODO
    [0x21] = z80_inst_ld_dd_nn,
    [0x22] = z80_inst_unimplemented,  // TODO
    [0x23] = z80_inst_inc_ss,
    [0x24] = z80_inst_inc_r,
    [0x25] = z80_inst_unimplemented,  // TODO
    [0x26] = z80_inst_ld_r_n,
    [0x27] = z80_inst_unimplemented,  // TODO
    [0x28] = z80_inst_unimplemented,  // TODO
    [0x29] = z80_inst_unimplemented,  // TODO
    [0x2A] = z80_inst_unimplemented,  // TODO
    [0x2B] = z80_inst_unimplemented,  // TODO
    [0x2C] = z80_inst_inc_r,
    [0x2D] = z80_inst_unimplemented,  // TODO
    [0x2E] = z80_inst_ld_r_n,
    [0x2F] = z80_inst_unimplemented,  // TODO
    [0x30] = z80_inst_unimplemented,  // TODO
    [0x31] = z80_inst_ld_dd_nn,
    [0x32] = z80_inst_unimplemented,  // TODO
    [0x33] = z80_inst_inc_ss,
    [0x34] = z80_inst_unimplemented,  // TODO
    [0x35] = z80_inst_unimplemented,  // TODO
    [0x36] = z80_inst_unimplemented,  // TODO
    [0x37] = z80_inst_unimplemented,  // TODO
    [0x38] = z80_inst_unimplemented,  // TODO
    [0x39] = z80_inst_unimplemented,  // TODO
    [0x3A] = z80_inst_unimplemented,  // TODO
    [0x3B] = z80_inst_unimplemented,  // TODO
    [0x3C] = z80_inst_inc_r,
    [0x3D] = z80_inst_unimplemented,  // TODO
    [0x3E] = z80_inst_ld_r_n,
    [0x3F] = z80_inst_unimplemented,  // TODO
    [0x40] = z80_inst_unimplemented,  // TODO
    [0x41] = z80_inst_unimplemented,  // TODO
    [0x42] = z80_inst_unimplemented,  // TODO
    [0x43] = z80_inst_unimplemented,  // TODO
    [0x44] = z80_inst_unimplemented,  // TODO
    [0x45] = z80_inst_unimplemented,  // TODO
    [0x46] = z80_inst_unimplemented,  // TODO
    [0x47] = z80_inst_unimplemented,  // TODO
    [0x48] = z80_inst_unimplemented,  // TODO
    [0x49] = z80_inst_unimplemented,  // TODO
    [0x4A] = z80_inst_unimplemented,  // TODO
    [0x4B] = z80_inst_unimplemented,  // TODO
    [0x4C] = z80_inst_unimplemented,  // TODO
    [0x4D] = z80_inst_unimplemented,  // TODO
    [0x4E] = z80_inst_unimplemented,  // TODO
    [0x4F] = z80_inst_unimplemented,  // TODO
    [0x50] = z80_inst_unimplemented,  // TODO
    [0x51] = z80_inst_unimplemented,  // TODO
    [0x52] = z80_inst_unimplemented,  // TODO
    [0x53] = z80_inst_unimplemented,  // TODO
    [0x54] = z80_inst_unimplemented,  // TODO
    [0x55] = z80_inst_unimplemented,  // TODO
    [0x56] = z80_inst_unimplemented,  // TODO
    [0x57] = z80_inst_unimplemented,  // TODO
    [0x58] = z80_inst_unimplemented,  // TODO
    [0x59] = z80_inst_unimplemented,  // TODO
    [0x5A] = z80_inst_unimplemented,  // TODO
    [0x5B] = z80_inst_unimplemented,  // TODO
    [0x5C] = z80_inst_unimplemented,  // TODO
    [0x5D] = z80_inst_unimplemented,  // TODO
    [0x5E] = z80_inst_unimplemented,  // TODO
    [0x5F] = z80_inst_unimplemented,  // TODO
    [0x60] = z80_inst_unimplemented,  // TODO
    [0x61] = z80_inst_unimplemented,  // TODO
    [0x62] = z80_inst_unimplemented,  // TODO
    [0x63] = z80_inst_unimplemented,  // TODO
    [0x64] = z80_inst_unimplemented,  // TODO
    [0x65] = z80_inst_unimplemented,  // TODO
    [0x66] = z80_inst_unimplemented,  // TODO
    [0x67] = z80_inst_unimplemented,  // TODO
    [0x68] = z80_inst_unimplemented,  // TODO
    [0x69] = z80_inst_unimplemented,  // TODO
    [0x6A] = z80_inst_unimplemented,  // TODO
    [0x6B] = z80_inst_unimplemented,  // TODO
    [0x6C] = z80_inst_unimplemented,  // TODO
    [0x6D] = z80_inst_unimplemented,  // TODO
    [0x6E] = z80_inst_unimplemented,  // TODO
    [0x6F] = z80_inst_unimplemented,  // TODO
    [0x70] = z80_inst_unimplemented,  // TODO
    [0x71] = z80_inst_unimplemented,  // TODO
    [0x72] = z80_inst_unimplemented,  // TODO
    [0x73] = z80_inst_unimplemented,  // TODO
    [0x74] = z80_inst_unimplemented,  // TODO
    [0x75] = z80_inst_unimplemented,  // TODO
    [0x76] = z80_inst_unimplemented,  // TODO
    [0x77] = z80_inst_unimplemented,  // TODO
    [0x78] = z80_inst_unimplemented,  // TODO
    [0x79] = z80_inst_unimplemented,  // TODO
    [0x7A] = z80_inst_unimplemented,  // TODO
    [0x7B] = z80_inst_unimplemented,  // TODO
    [0x7C] = z80_inst_unimplemented,  // TODO
    [0x7D] = z80_inst_unimplemented,  // TODO
    [0x7E] = z80_inst_unimplemented,  // TODO
    [0x7F] = z80_inst_unimplemented,  // TODO
    [0x80] = z80_inst_unimplemented,  // TODO
    [0x81] = z80_inst_unimplemented,  // TODO
    [0x82] = z80_inst_unimplemented,  // TODO
    [0x83] = z80_inst_unimplemented,  // TODO
    [0x84] = z80_inst_unimplemented,  // TODO
    [0x85] = z80_inst_unimplemented,  // TODO
    [0x86] = z80_inst_unimplemented,  // TODO
    [0x87] = z80_inst_unimplemented,  // TODO
    [0x88] = z80_inst_unimplemented,  // TODO
    [0x89] = z80_inst_unimplemented,  // TODO
    [0x8A] = z80_inst_unimplemented,  // TODO
    [0x8B] = z80_inst_unimplemented,  // TODO
    [0x8C] = z80_inst_unimplemented,  // TODO
    [0x8D] = z80_inst_unimplemented,  // TODO
    [0x8E] = z80_inst_unimplemented,  // TODO
    [0x8F] = z80_inst_unimplemented,  // TODO
    [0x90] = z80_inst_unimplemented,  // TODO
    [0x91] = z80_inst_unimplemented,  // TODO
    [0x92] = z80_inst_unimplemented,  // TODO
    [0x93] = z80_inst_unimplemented,  // TODO
    [0x94] = z80_inst_unimplemented,  // TODO
    [0x95] = z80_inst_unimplemented,  // TODO
    [0x96] = z80_inst_unimplemented,  // TODO
    [0x97] = z80_inst_unimplemented,  // TODO
    [0x98] = z80_inst_unimplemented,  // TODO
    [0x99] = z80_inst_unimplemented,  // TODO
    [0x9A] = z80_inst_unimplemented,  // TODO
    [0x9B] = z80_inst_unimplemented,  // TODO
    [0x9C] = z80_inst_unimplemented,  // TODO
    [0x9D] = z80_inst_unimplemented,  // TODO
    [0x9E] = z80_inst_unimplemented,  // TODO
    [0x9F] = z80_inst_unimplemented,  // TODO
    [0xA0] = z80_inst_unimplemented,  // TODO
    [0xA1] = z80_inst_unimplemented,  // TODO
    [0xA2] = z80_inst_unimplemented,  // TODO
    [0xA3] = z80_inst_unimplemented,  // TODO
    [0xA4] = z80_inst_unimplemented,  // TODO
    [0xA5] = z80_inst_unimplemented,  // TODO
    [0xA6] = z80_inst_unimplemented,  // TODO
    [0xA7] = z80_inst_unimplemented,  // TODO
    [0xA8] = z80_inst_unimplemented,  // TODO
    [0xA9] = z80_inst_unimplemented,  // TODO
    [0xAA] = z80_inst_unimplemented,  // TODO
    [0xAB] = z80_inst_unimplemented,  // TODO
    [0xAC] = z80_inst_unimplemented,  // TODO
    [0xAD] = z80_inst_unimplemented,  // TODO
    [0xAE] = z80_inst_unimplemented,  // TODO
    [0xAF] = z80_inst_unimplemented,  // TODO
    [0xB0] = z80_inst_unimplemented,  // TODO
    [0xB1] = z80_inst_unimplemented,  // TODO
    [0xB2] = z80_inst_unimplemented,  // TODO
    [0xB3] = z80_inst_unimplemented,  // TODO
    [0xB4] = z80_inst_unimplemented,  // TODO
    [0xB5] = z80_inst_unimplemented,  // TODO
    [0xB6] = z80_inst_unimplemented,  // TODO
    [0xB7] = z80_inst_unimplemented,  // TODO
    [0xB8] = z80_inst_unimplemented,  // TODO
    [0xB9] = z80_inst_unimplemented,  // TODO
    [0xBA] = z80_inst_unimplemented,  // TODO
    [0xBB] = z80_inst_unimplemented,  // TODO
    [0xBC] = z80_inst_unimplemented,  // TODO
    [0xBD] = z80_inst_unimplemented,  // TODO
    [0xBE] = z80_inst_unimplemented,  // TODO
    [0xBF] = z80_inst_unimplemented,  // TODO
    [0xC0] = z80_inst_unimplemented,  // TODO
    [0xC1] = z80_inst_unimplemented,  // TODO
    [0xC2] = z80_inst_unimplemented,  // TODO
    [0xC3] = z80_inst_unimplemented,  // TODO
    [0xC4] = z80_inst_unimplemented,  // TODO
    [0xC5] = z80_inst_unimplemented,  // TODO
    [0xC6] = z80_inst_unimplemented,  // TODO
    [0xC7] = z80_inst_unimplemented,  // TODO
    [0xC8] = z80_inst_unimplemented,  // TODO
    [0xC9] = z80_inst_unimplemented,  // TODO
    [0xCA] = z80_inst_unimplemented,  // TODO
    [0xCB] = z80_inst_unimplemented,  // TODO
    [0xCC] = z80_inst_unimplemented,  // TODO
    [0xCD] = z80_inst_unimplemented,  // TODO
    [0xCE] = z80_inst_unimplemented,  // TODO
    [0xCF] = z80_inst_unimplemented,  // TODO
    [0xD0] = z80_inst_unimplemented,  // TODO
    [0xD1] = z80_inst_unimplemented,  // TODO
    [0xD2] = z80_inst_unimplemented,  // TODO
    [0xD3] = z80_inst_unimplemented,  // TODO
    [0xD4] = z80_inst_unimplemented,  // TODO
    [0xD5] = z80_inst_unimplemented,  // TODO
    [0xD6] = z80_inst_unimplemented,  // TODO
    [0xD7] = z80_inst_unimplemented,  // TODO
    [0xD8] = z80_inst_unimplemented,  // TODO
    [0xD9] = z80_inst_unimplemented,  // TODO
    [0xDA] = z80_inst_unimplemented,  // TODO
    [0xDB] = z80_inst_unimplemented,  // TODO
    [0xDC] = z80_inst_unimplemented,  // TODO
    [0xDD] = z80_inst_unimplemented,  // TODO
    [0xDE] = z80_inst_unimplemented,  // TODO
    [0xDF] = z80_inst_unimplemented,  // TODO
    [0xE0] = z80_inst_unimplemented,  // TODO
    [0xE1] = z80_inst_unimplemented,  // TODO
    [0xE2] = z80_inst_unimplemented,  // TODO
    [0xE3] = z80_inst_unimplemented,  // TODO
    [0xE4] = z80_inst_unimplemented,  // TODO
    [0xE5] = z80_inst_unimplemented,  // TODO
    [0xE6] = z80_inst_unimplemented,  // TODO
    [0xE7] = z80_inst_unimplemented,  // TODO
    [0xE8] = z80_inst_unimplemented,  // TODO
    [0xE9] = z80_inst_unimplemented,  // TODO
    [0xEA] = z80_inst_unimplemented,  // TODO
    [0xEB] = z80_inst_unimplemented,  // TODO
    [0xEC] = z80_inst_unimplemented,  // TODO
    [0xED] = z80_inst_unimplemented,  // TODO
    [0xEE] = z80_inst_unimplemented,  // TODO
    [0xEF] = z80_inst_unimplemented,  // TODO
    [0xF0] = z80_inst_unimplemented,  // TODO
    [0xF1] = z80_inst_unimplemented,  // TODO
    [0xF2] = z80_inst_unimplemented,  // TODO
    [0xF3] = z80_inst_unimplemented,  // TODO
    [0xF4] = z80_inst_unimplemented,  // TODO
    [0xF5] = z80_inst_unimplemented,  // TODO
    [0xF6] = z80_inst_unimplemented,  // TODO
    [0xF7] = z80_inst_unimplemented,  // TODO
    [0xF8] = z80_inst_unimplemented,  // TODO
    [0xF9] = z80_inst_unimplemented,  // TODO
    [0xFA] = z80_inst_unimplemented,  // TODO
    [0xFB] = z80_inst_unimplemented,  // TODO
    [0xFC] = z80_inst_unimplemented,  // TODO
    [0xFD] = z80_inst_unimplemented,  // TODO
    [0xFE] = z80_inst_unimplemented,  // TODO
    [0xFF] = z80_inst_unimplemented   // TODO
};
