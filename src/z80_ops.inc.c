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

typedef uint8_t (*DispatchTable[256])(Z80*, uint8_t);

static DispatchTable instruction_table;
static DispatchTable instruction_table_extended;
static DispatchTable instruction_table_bits;
static DispatchTable instruction_table_index;
static DispatchTable instruction_table_index_bits;

/*
    Unimplemented opcode handler.
*/
static uint8_t z80_inst_unimplemented(Z80 *z80, uint8_t opcode)
{
    z80->except = true;
    z80->exc_code = Z80_EXC_UNIMPLEMENTED_OPCODE;
    z80->exc_data = opcode;
    return 4;
}

/*
    LD r, r' (0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x47, 0x48, 0x49, 0x4A, 0x4B,
        0x4C, 0x4D, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x57, 0x58, 0x59,
        0x5A, 0x5B, 0x5C, 0x5D, 0x5F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x67,
        0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6F, 0x78, 0x79, 0x7A, 0x7B, 0x7C,
        0x7D, 0x7F):
    Load r' (8-bit register) into r (8-bit register).
*/
static uint8_t z80_inst_ld_r_r(Z80 *z80, uint8_t opcode)
{
    uint8_t *dst = extract_reg(z80, opcode),
            *src = extract_reg(z80, opcode << 3);
    *dst = *src;
    z80->regfile.pc++;
    return 4;
}

/*
    LD r, n (0x06, 0x0E, 0x16, 0x1E, 0x26, 0x2E, 0x3E):
    Load n (8-bit immediate) into r (8-bit register).
*/
static uint8_t z80_inst_ld_r_n(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg = extract_reg(z80, opcode);
    *reg = mmu_read_byte(z80->mmu, ++z80->regfile.pc);
    z80->regfile.pc++;
    return 7;
}

/*
    LD r, (HL) (0x46, 0x4E, 0x56, 0x5E, 0x66, 0x6E, 0x7E):
    Load the contents of HL into r (8-bit register).
*/
static uint8_t z80_inst_ld_r_hl(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg = extract_reg(z80, opcode);
    *reg = mmu_read_byte(z80->mmu, get_pair(z80, REG_HL));
    z80->regfile.pc++;
    return 7;
}

/*
    LD r, (IX+d)
*/
// static uint8_t z80_inst_ld_r_ix(Z80 *z80, uint8_t opcode)

/*
    LD r, (IY+d)
*/
// static uint8_t z80_inst_ld_r_iy(Z80 *z80, uint8_t opcode)

/*
    LD (HL), r
*/
// static uint8_t z80_inst_ld_hl_r(Z80 *z80, uint8_t opcode)

/*
    LD (IX+d), r
*/
// static uint8_t z80_inst_ld_ix_r(Z80 *z80, uint8_t opcode)

/*
    LD (IY+d), r
*/
// static uint8_t z80_inst_ld_iy_r(Z80 *z80, uint8_t opcode)

/*
    LD (HL), n
*/
// static uint8_t z80_inst_ld_hl_n(Z80 *z80, uint8_t opcode)

/*
    LD (IX+d), n
*/
// static uint8_t z80_inst_ld_ix_n(Z80 *z80, uint8_t opcode)

/*
    LD (IY+d), n
*/
// static uint8_t z80_inst_ld_iy_n(Z80 *z80, uint8_t opcode)

/*
    LD A, (BC)
*/
// static uint8_t z80_inst_ld_a_bc(Z80 *z80, uint8_t opcode)

/*
    LD A, (DE)
*/
// static uint8_t z80_inst_ld_a_de(Z80 *z80, uint8_t opcode)

/*
    LD A, (nn)
*/
// static uint8_t z80_inst_ld_a_nn(Z80 *z80, uint8_t opcode)

/*
    LD (BC), A
*/
// static uint8_t z80_inst_ld_bc_a(Z80 *z80, uint8_t opcode)

/*
    LD (DE), A
*/
// static uint8_t z80_inst_ld_de_a(Z80 *z80, uint8_t opcode)

/*
    LD (nn), A
*/
// static uint8_t z80_inst_ld_nn_a(Z80 *z80, uint8_t opcode)

/*
    LD A, I
*/
// static uint8_t z80_inst_ld_a_i(Z80 *z80, uint8_t opcode)

/*
    LD A, R
*/
// static uint8_t z80_inst_ld_a_r(Z80 *z80, uint8_t opcode)

/*
    LD I,A
*/
// static uint8_t z80_inst_ld_i_a(Z80 *z80, uint8_t opcode)

/*
    LD R, A
*/
// static uint8_t z80_inst_ld_r_a(Z80 *z80, uint8_t opcode)

/*
    LD dd, nn (0x01, 0x11, 0x21, 0x31):
    Load nn (16-bit immediate) into dd (16-bit register).
*/
static uint8_t z80_inst_ld_dd_nn(Z80 *z80, uint8_t opcode)
{
    uint8_t pair = extract_pair(opcode);
    set_pair(z80, pair, mmu_read_double(z80->mmu, ++z80->regfile.pc));
    z80->regfile.pc += 2;
    return 10;
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

/*
    EXX (0xD9):
    Exchange the 16-bit registers with their shadows
    (BC <=> BC', DE <=> DE', HL <=> HL').
*/
static uint8_t z80_inst_exx(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t bc = get_pair(z80, REG_BC),
             de = get_pair(z80, REG_DE),
             hl = get_pair(z80, REG_HL);

    set_pair(z80, REG_BC, get_pair(z80, REG_BC_));
    set_pair(z80, REG_DE, get_pair(z80, REG_DE_));
    set_pair(z80, REG_HL, get_pair(z80, REG_HL_));

    set_pair(z80, REG_BC_, bc);
    set_pair(z80, REG_DE_, de);
    set_pair(z80, REG_HL_, hl);
    z80->regfile.pc++;
    return 4;
}

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
    Increment r (8-bit register).
*/
static uint8_t z80_inst_inc_r(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg = extract_reg(z80, opcode);
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

/*
    HALT (0x76):
    Suspend CPU operation: execute NOPs until an interrupt or reset.
*/
static uint8_t z80_inst_halt(Z80 *z80, uint8_t opcode)
{
    (void) z80;
    (void) opcode;
    return 4;
}

/*
    DI (0xF3):
    Disable maskable interrupts by resetting both flip-flops.
*/
static uint8_t z80_inst_di(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    z80->regfile.iff1 = false;
    z80->regfile.iff2 = false;
    z80->regfile.pc++;
    return 4;
}

/*
    EI (0xFB):
    Enable maskable interrupts by setting both flip-flops.
*/
static uint8_t z80_inst_ei(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    z80->regfile.iff1 = true;
    z80->regfile.iff2 = true;
    z80->regfile.pc++;
    return 4;
}

/*
    IM (0xED46, 0xED4E, 0xED56, 0xED5E, 0xED66, 0xED6E, 0xED76, 0xED7E):
    Set the interrupt mode.
*/
static uint8_t z80_inst_im(Z80 *z80, uint8_t opcode)
{
    switch (opcode) {
        case 0x46:
        case 0x4E:
        case 0x66:
        case 0x6E:
            z80->regfile.im_a = false;  // Interrupt mode 0
            z80->regfile.im_b = false;
            break;
        case 0x56:
        case 0x76:
            z80->regfile.im_a = true;  // Interrupt mode 1
            z80->regfile.im_b = false;
            break;
        case 0x5E:
        case 0x7E:
            z80->regfile.im_a = true;  // Interrupt mode 2
            z80->regfile.im_b = true;
            break;
    }

    z80->regfile.pc++;
    return 8;
}

// ADD HL, ss

// ADC HL, ss

// SBC HL, ss

// ADD IX, pp

// ADD IY, rr

/*
    INC ss (0x03, 0x13, 0x23, 0x33):
    Increment ss (16-bit register).
*/
static uint8_t z80_inst_inc_ss(Z80 *z80, uint8_t opcode)
{
    uint8_t pair = extract_pair(opcode);
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

/*
    JP nn (0xC3):
    Jump to nn (16-bit immediate).
*/
static uint8_t z80_inst_jp_nn(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    z80->regfile.pc = mmu_read_double(z80->mmu, ++z80->regfile.pc);
    return 10;
}

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

/*
    0xED:
    Handle an extended instruction.
*/
static uint8_t z80_prefix_extended(Z80 *z80, uint8_t opcode)
{
    opcode = mmu_read_byte(z80->mmu, ++z80->regfile.pc);
    return (*instruction_table_extended[opcode])(z80, opcode);
}

/*
    0xED:
    Handle a bit instruction.
*/
static uint8_t z80_prefix_bits(Z80 *z80, uint8_t opcode)
{
    opcode = mmu_read_byte(z80->mmu, ++z80->regfile.pc);
    return (*instruction_table_bits[opcode])(z80, opcode);
}

/*
    0xDD, 0xFD:
    Handle an index instruction.
*/
static uint8_t z80_prefix_index(Z80 *z80, uint8_t opcode)
{
    opcode = mmu_read_byte(z80->mmu, ++z80->regfile.pc);
    return (*instruction_table_index[opcode])(z80, opcode);
}

/*
    0xDDCB, 0xFDCB:
    Handle an index-bit instruction.
*/
static uint8_t z80_prefix_index_bits(Z80 *z80, uint8_t opcode)
{
    opcode = mmu_read_byte(z80->mmu, ++z80->regfile.pc);
    return (*instruction_table_index_bits[opcode])(z80, opcode);
}

#include "z80_tables.inc.c"
