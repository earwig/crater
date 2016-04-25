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
    Load the memory pointed to by HL into r (8-bit register).
*/
static uint8_t z80_inst_ld_r_hl(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg = extract_reg(z80, opcode);
    *reg = mmu_read_byte(z80->mmu, get_pair(z80, REG_HL));
    z80->regfile.pc++;
    return 7;
}

/*
    LD r, (IXY+d)
*/
// static uint8_t z80_inst_ld_r_ixy(Z80 *z80, uint8_t opcode)

/*
    LD (HL), r (0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x77):
    Load r (8-bit register) into the memory pointed to by HL.
*/
static uint8_t z80_inst_ld_hl_r(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg = extract_reg(z80, opcode << 3);
    uint16_t addr = get_pair(z80, REG_HL);
    mmu_write_byte(z80->mmu, addr, *reg);
    z80->regfile.pc++;
    return 7;
}

/*
    LD (IXY+d), r
*/
// static uint8_t z80_inst_ld_ixy_r(Z80 *z80, uint8_t opcode)

/*
    LD (HL), n (0x36):
    Load n (8-bit immediate) into the memory address pointed to by HL.
*/
static uint8_t z80_inst_ld_hl_n(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t addr = get_pair(z80, REG_HL);
    uint8_t byte = mmu_read_byte(z80->mmu, ++z80->regfile.pc);
    mmu_write_byte(z80->mmu, addr, byte);
    z80->regfile.pc++;
    return 10;
}

/*
    LD (IXY+d), n
*/
// static uint8_t z80_inst_ld_ixy_n(Z80 *z80, uint8_t opcode)

/*
    LD A, (BC/DE)
*/
// static uint8_t z80_inst_ld_a_bcde(Z80 *z80, uint8_t opcode)

/*
    LD A, (nn) (0x3A):
    Load memory at address nn into A.
*/
static uint8_t z80_inst_ld_a_nn(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t addr = mmu_read_double(z80->mmu, ++z80->regfile.pc);
    z80->regfile.a = mmu_read_byte(z80->mmu, addr);
    z80->regfile.pc += 2;
    return 13;
}

/*
    LD (BC/DE), A (0x02, 0x12):
    Load A into the memory address pointed to by BC or DE.
*/
static uint8_t z80_inst_ld_bcde_a(Z80 *z80, uint8_t opcode)
{
    uint16_t addr = get_pair(z80, extract_pair(opcode));
    mmu_write_byte(z80->mmu, addr, z80->regfile.a);
    z80->regfile.pc++;
    return 7;
}

/*
    LD (nn), A (0x32):
    Load A into memory address nn.
*/
static uint8_t z80_inst_ld_nn_a(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t addr = mmu_read_double(z80->mmu, ++z80->regfile.pc);
    mmu_write_byte(z80->mmu, addr, z80->regfile.a);
    z80->regfile.pc += 2;
    return 13;
}

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

// LD IXY, nn

// LD HL, (nn)

// LD dd, (nn)

// LD IXY, (nn)

/*
    LD (nn), HL: (0x22):
    Load HL into memory address nn.
*/
static uint8_t z80_inst_ld_nn_hl(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t addr = mmu_read_double(z80->mmu, ++z80->regfile.pc);
    mmu_write_double(z80->mmu, addr, get_pair(z80, REG_HL));
    z80->regfile.pc += 2;
    return 16;
}

// LD (nn), dd

// LD (nn), IXY

// LD SP, HL

// LD SP, IXY

/*
    PUSH qq (0xC5, 0xD5, 0xE5, 0xF5):
    Push qq onto the stack, and decrement SP by two.
*/
static uint8_t z80_inst_push_qq(Z80 *z80, uint8_t opcode)
{
    uint8_t pair = extract_pair(opcode);
    stack_push(z80, get_pair(z80, pair));
    z80->regfile.pc++;
    return 11;
}

/*
    PUSH IXY (0xDDE5, 0xFDE5):
    Push IX or IY onto the stack, and decrement SP by two.
*/
static uint8_t z80_inst_push_ixy(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    stack_push(z80, *extract_index(z80));
    z80->regfile.pc++;
    return 15;
}

/*
    POP qq (0xC1, 0xD1, 0xE1, 0xF1):
    Pop qq from the stack, and increment SP by two.
*/
static uint8_t z80_inst_pop_qq(Z80 *z80, uint8_t opcode)
{
    uint8_t pair = extract_pair(opcode);
    set_pair(z80, pair, stack_pop(z80));
    z80->regfile.pc++;
    return 10;
}

/*
    POP IXY (0xDDE1, 0xFDE1):
    Pop IX or IY from the stack, and increment SP by two.
*/
static uint8_t z80_inst_pop_ixy(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    *extract_index(z80) = stack_pop(z80);
    z80->regfile.pc++;
    return 14;
}

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

// EX (SP), IXY

/*
    LDI (0xEDA0):
    LD (DE), (HL); INC HL; INC DE; DEC BC;
*/
static uint8_t z80_inst_ldi(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t hl = get_pair(z80, REG_HL),
             de = get_pair(z80, REG_DE),
             bc = get_pair(z80, REG_BC);
    uint16_t value = mmu_read_double(z80->mmu, hl);

    mmu_write_double(z80->mmu, de, value);
    set_pair(z80, REG_HL, ++hl);
    set_pair(z80, REG_DE, ++de);
    set_pair(z80, REG_BC, --bc);

    update_flags(z80, 0, 0, bc == 0, 0, 0, 0, 0, 0, 0x16);
    z80->regfile.pc++;
    return 16;
}

/*
    LDIR (0xEDB0):
    LDI; JR PV, -2
*/
static uint8_t z80_inst_ldir(Z80 *z80, uint8_t opcode)
{
    z80_inst_ldi(z80, opcode);
    if (get_pair(z80, REG_BC) == 0)
        return 16;
    z80->regfile.pc -= 2;
    return 21;
}

/*
    LDD (0xEDA8):
    LD (DE), (HL); DEC HL; DEC DE; DEC BC;
*/
static uint8_t z80_inst_ldd(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t hl = get_pair(z80, REG_HL),
             de = get_pair(z80, REG_DE),
             bc = get_pair(z80, REG_BC);
    uint16_t value = mmu_read_double(z80->mmu, hl);

    mmu_write_double(z80->mmu, de, value);
    set_pair(z80, REG_HL, --hl);
    set_pair(z80, REG_DE, --de);
    set_pair(z80, REG_BC, --bc);

    update_flags(z80, 0, 0, bc == 0, 0, 0, 0, 0, 0, 0x16);
    z80->regfile.pc++;
    return 16;
}

/*
    LDDR (0xEDB8):
    LDD; JR PV, -2
*/
static uint8_t z80_inst_lddr(Z80 *z80, uint8_t opcode)
{
    z80_inst_ldd(z80, opcode);
    if (get_pair(z80, REG_BC) == 0)
        return 16;
    z80->regfile.pc -= 2;
    return 21;
}

// CPI

// CPIR

// CPD

// CPDR

// ADD A, r

// ADD A, n

// ADD A, (HL)

// ADD A, (IXY+d)

// ADC A, s

// SUB s

// SBC A, s

// AND s

// OR s

/*
    OR r (0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB7):
    Bitwise OR a with r (8-bit register).
*/
static uint8_t z80_inst_or_r(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg = extract_reg(z80, opcode << 3);
    uint8_t a = (z80->regfile.a ^= *reg);

    bool parity = !(__builtin_popcount(a) % 2);
    update_flags(z80, 0, 0, parity, !!(a & 0x08), 0, !!(a & 0x20), a == 0,
                 !!(a & 0x80), 0xFF);

    z80->regfile.pc++;
    return 4;
}

// XOR s

/*
    XOR r (0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAF):
    Bitwise XOR a with r (8-bit register).
*/
static uint8_t z80_inst_xor_r(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg = extract_reg(z80, opcode << 3);
    uint8_t a = (z80->regfile.a ^= *reg);

    bool parity = !(__builtin_popcount(a) % 2);
    update_flags(z80, 0, 0, parity, !!(a & 0x08), 0, !!(a & 0x20), a == 0,
                 !!(a & 0x80), 0xFF);

    z80->regfile.pc++;
    return 4;
}

// CP s

/*
    CP r (0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBF):
    Set flags as if r (8-bit register) had been subtracted from a.
*/
static uint8_t z80_inst_cp_r(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg = extract_reg(z80, opcode << 3);
    uint8_t d = z80->regfile.a - *reg;

    bool c = (z80->regfile.a - *reg) != d;
    bool v = (z80->regfile.a - *reg) != ((int8_t) d);
    bool h = !!(((z80->regfile.a & 0x0F) - (*reg & 0x0F)) & 0x10);
    update_flags(z80, c, 1, v, !!(*reg & 0x08), h, !!(*reg & 0x20), d == 0,
                 !!(d & 0x80), 0xFF);

    z80->regfile.pc++;
    return 4;
}

/*
    CP n (0xFE):
    Set flags as if n (8-bit immediate) had been subtracted from a.
*/
static uint8_t z80_inst_cp_n(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t n = mmu_read_byte(z80->mmu, ++z80->regfile.pc);
    uint8_t d = z80->regfile.a - n;

    bool c = (z80->regfile.a - n) != d;
    bool v = (z80->regfile.a - n) != ((int8_t) d);
    bool h = !!(((z80->regfile.a & 0x0F) - (n & 0x0F)) & 0x10);
    update_flags(z80, c, 1, v, !!(n & 0x08), h, !!(n & 0x20), d == 0,
                 !!(d & 0x80), 0xFF);

    z80->regfile.pc++;
    return 7;
}

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

// INC (IXY+d)

/*
    DEC r (0x05, 0x0D, 0x15, 0x1D, 0x25, 0x2D, 0x3D):
    Decrement r (8-bit register).
*/
static uint8_t z80_inst_dec_r(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg = extract_reg(z80, opcode);
    bool halfcarry = !!(((*reg & 0x0F) - 1) & 0x10);
    (*reg)--;
    update_flags(z80, 0, 1, *reg == 0x7F, !!(*reg & 0x08), halfcarry,
                 !!(*reg & 0x20), *reg == 0, !!(*reg & 0x80), 0xFE);

    z80->regfile.pc++;
    return 4;
}

// DEC (HL)

// DEC (IXY+d)

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

/*
    SBC HL, ss (0xED42, 0xED52, 0xED62, 0xED72):
    Subtract BC with carry from HL.
*/
static uint8_t z80_inst_sbc_hl_ss(Z80 *z80, uint8_t opcode)
{
    uint8_t pair = extract_pair(opcode);
    uint16_t minuend = get_pair(z80, REG_HL);
    uint16_t subtrahend = get_pair(z80, pair) + get_flag(z80, FLAG_CARRY);
    uint16_t value = minuend - subtrahend;
    set_pair(z80, REG_HL, value);

    bool c  = (minuend - subtrahend) != value;
    bool ov = (minuend - subtrahend) != ((int16_t) value);  // TODO: verify these
    bool hc = !!(((minuend & 0x0FFF) - (subtrahend & 0x0FFF)) & 0x1000);

    update_flags(z80, c, 1, ov, !!(value & 0x0800), hc,
                 !!(value & 0x2000), value == 0, !!(value & 0x8000), 0xFF);

    z80->regfile.pc++;
    return 15;
}

// ADD IXY, pp

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

// INC IXY

/*
    DEC ss (0x0B, 0x1B, 0x2B, 0x3B):
    Decrement ss (16-bit register).
*/
static uint8_t z80_inst_dec_ss(Z80 *z80, uint8_t opcode)
{
    uint8_t pair = extract_pair(opcode);
    set_pair(z80, pair, get_pair(z80, pair) - 1);
    z80->regfile.pc++;
    return 6;
}

// DEC IXY

// RLCA

// RLA

// RRCA

// RRA

// RLC r

// RLC (HL)

// RLC (IXY+d)

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

// BIT b, (IXY+d)

// SET b, r

// SET b, (HL)

// SET b, (IXY+d)

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

/*
    JP cc, nn (0xC2, 0xCA, 0xD2, 0xDA, 0xE2, 0xEA, 0xF2, 0xFA):
    Jump to nn (16-bit immediate) if cc (condition) is true.
*/
static uint8_t z80_inst_jp_cc_nn(Z80 *z80, uint8_t opcode)
{
    if (extract_cond(z80, opcode))
        z80->regfile.pc = mmu_read_double(z80->mmu, ++z80->regfile.pc);
    else
        z80->regfile.pc += 3;
    return 10;
}

/*
    JR e (0x18):
    Relative jump e (signed 8-bit immediate) bytes.
*/
static uint8_t z80_inst_jr_e(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    int8_t jump = mmu_read_byte(z80->mmu, z80->regfile.pc + 1);
    z80->regfile.pc += jump + 2;
    return 12;
}


/*
    JR cc, e (0x20, 0x28, 0x30, 0x38):
    Relative jump e (signed 8-bit immediate) bytes if cc (condition) is true.
*/
static uint8_t z80_inst_jr_cc_e(Z80 *z80, uint8_t opcode)
{
    if (extract_cond(z80, opcode - 0x20)) {
        int8_t jump = mmu_read_byte(z80->mmu, z80->regfile.pc + 1);
        z80->regfile.pc += jump + 2;
        return 12;
    } else {
        z80->regfile.pc += 2;
        return 7;
    }
}

// JP (HL)

// JP (IXY)

/*
    DJNZ, e (0x10):
    Decrement b and relative jump e (signed 8-bit immediate) if non-zero.
*/
static uint8_t z80_inst_djnz_e(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    z80->regfile.b--;

    if (z80->regfile.b != 0) {
        int8_t jump = mmu_read_byte(z80->mmu, z80->regfile.pc + 1);
        z80->regfile.pc += jump + 2;
        return 13;
    } else {
        z80->regfile.pc += 2;
        return 8;
    }
}

/*
    CALL nn (0xCD):
    Push PC+3 onto the stack and jump to nn (16-bit immediate).
*/
static uint8_t z80_inst_call_nn(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    stack_push(z80, z80->regfile.pc + 3);
    z80->regfile.pc = mmu_read_double(z80->mmu, ++z80->regfile.pc);
    return 17;
}

/*
    CALL cc, nn (0xC4, 0xCC, 0xD4, 0xDC, 0xE4, 0xEC, 0xF4, 0xFC):
    Push PC+3 onto the stack and jump to nn (16-bit immediate) if cc is true.
*/
static uint8_t z80_inst_call_cc_nn(Z80 *z80, uint8_t opcode)
{
    if (extract_cond(z80, opcode)) {
        stack_push(z80, z80->regfile.pc + 3);
        z80->regfile.pc = mmu_read_double(z80->mmu, ++z80->regfile.pc);
        return 17;
    } else {
        z80->regfile.pc += 3;
        return 10;
    }
}

/*
    RET (0xC9):
    Pop PC from the stack.
*/
static uint8_t z80_inst_ret(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    z80->regfile.pc = stack_pop(z80);
    return 10;
}

/*
    RET cc (0xC0, 0xC8, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8):
    Pop PC from the stack if cc is true.
*/
static uint8_t z80_inst_ret_cc(Z80 *z80, uint8_t opcode)
{
    if (extract_cond(z80, opcode)) {
        z80->regfile.pc = stack_pop(z80);
        return 11;
    } else {
        z80->regfile.pc++;
        return 5;
    }
}

// RETI

/*
    RETN (0xED45, 0xED55, 0xED5D, 0xED65, 0xED6D, 0xED75, 0xED7D):
    Pop PC from the stack, and copy to IFF2 to IFF1.
*/
static uint8_t z80_inst_retn(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    z80->regfile.pc = stack_pop(z80);
    z80->regfile.iff1 = z80->regfile.iff2;
    return 14;
}

/*
    RST p (0xC7, 0xCF, 0xD7, 0xDF, 0xE7, 0xEF, 0xF7, 0xFF):
    Push PC+1 onto the stack and jump to p (opcode & 0x38).
*/
static uint8_t z80_inst_rst_p(Z80 *z80, uint8_t opcode)
{
    stack_push(z80, z80->regfile.pc + 1);
    z80->regfile.pc = opcode & 0x38;
    return 11;
}

/*
    IN A, (n) (0xDB):
    Read a byte from port n into a.
*/
static uint8_t z80_inst_in_a_n(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t port = mmu_read_byte(z80->mmu, ++z80->regfile.pc);
    z80->regfile.a = read_port(z80, port);
    z80->regfile.pc++;
    return 11;
}

/*
    IN r, (C) (0xED40, 0xED48, 0xED50, 0xED58, 0xED60, 0xED68, 0xED70, 0xED78):
    Read a byte from port C into r, or affect flags only if 0xED70.
*/
static uint8_t z80_inst_in_r_c(Z80 *z80, uint8_t opcode)
{
    uint8_t data = read_port(z80, z80->regfile.c);
    bool parity = !(__builtin_popcount(data) % 2);

    if (opcode != 0x70)
        *extract_reg(z80, opcode) = data;
    update_flags(z80, 0, 0, parity, !!(data & 0x08), 0, !!(data & 0x20),
        data == 0, !!(data & 0x80), 0xFE);
    z80->regfile.pc++;
    return 12;
}

/*
    INI (0xEDA2):
    IN (HL), (C); INC HL; DEC B
*/
static uint8_t z80_inst_ini(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t data = read_port(z80, z80->regfile.c), *b = &z80->regfile.b;
    uint16_t hl = get_pair(z80, REG_HL);
    bool h = !!(((*b & 0x0F) - 1) & 0x10);

    mmu_write_byte(z80->mmu, hl, data);
    set_pair(z80, REG_HL, hl + 1);
    (*b)--;

    update_flags(z80, 0, 1, *b == 0x7F, !!(*b & 0x08), h, !!(*b & 0x20),
        *b == 0, !!(*b & 0x80), 0xFE);
    z80->regfile.pc++;
    return 16;
}

/*
    INIR (0xEDB2):
    INI; JR NZ, -2
*/
static uint8_t z80_inst_inir(Z80 *z80, uint8_t opcode)
{
    z80_inst_ini(z80, opcode);
    if (z80->regfile.b == 0)
        return 16;
    z80->regfile.pc -= 2;
    return 21;
}

/*
    IND (0xEDAA):
    IN (HL), (C); DEC HL; DEC B
*/
static uint8_t z80_inst_ind(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t data = read_port(z80, z80->regfile.c), *b = &z80->regfile.b;
    uint16_t hl = get_pair(z80, REG_HL);
    bool h = !!(((*b & 0x0F) - 1) & 0x10);

    mmu_write_byte(z80->mmu, hl, data);
    set_pair(z80, REG_HL, hl - 1);
    (*b)--;

    update_flags(z80, 0, 1, *b == 0x7F, !!(*b & 0x08), h, !!(*b & 0x20),
        *b == 0, !!(*b & 0x80), 0xFE);
    z80->regfile.pc++;
    return 16;
}

/*
    INDR (0xEDBA):
    IND; JR NZ, -2
*/
static uint8_t z80_inst_indr(Z80 *z80, uint8_t opcode)
{
    z80_inst_ind(z80, opcode);
    if (z80->regfile.b == 0)
        return 16;
    z80->regfile.pc -= 2;
    return 21;
}

/*
    OUT (n), A (0xD3):
    Write a byte from a into port n.
*/
static uint8_t z80_inst_out_n_a(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t port = mmu_read_byte(z80->mmu, ++z80->regfile.pc);
    write_port(z80, port, z80->regfile.a);
    z80->regfile.pc++;
    return 11;
}

/*
    OUT (C), r (0xED41, 0xED49, 0xED51, 0xED59, 0xED61, 0xED69, 0xED71,
        0xED79):
    Write a byte from r (8-bit reg, or 0 if 0xED71) into port C.
*/
static uint8_t z80_inst_out_c_r(Z80 *z80, uint8_t opcode)
{
    uint8_t value = opcode != 0x71 ? *extract_reg(z80, opcode) : 0;
    write_port(z80, z80->regfile.c, value);
    z80->regfile.pc++;
    return 12;
}

/*
    OUTI (0xEDA3):
    OUT (C), (HL); INC HL; DEC B
*/
static uint8_t z80_inst_outi(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t hl = get_pair(z80, REG_HL);
    uint8_t *b = &z80->regfile.b;
    bool h = !!(((*b & 0x0F) - 1) & 0x10);

    write_port(z80, z80->regfile.c, mmu_read_byte(z80->mmu, hl));
    set_pair(z80, REG_HL, hl + 1);
    (*b)--;

    update_flags(z80, 0, 1, *b == 0x7F, !!(*b & 0x08), h, !!(*b & 0x20),
        *b == 0, !!(*b & 0x80), 0xFE);
    z80->regfile.pc++;
    return 16;
}

/*
    OTIR (0xEDB3):
    OUTI; JR NZ, -2
*/
static uint8_t z80_inst_otir(Z80 *z80, uint8_t opcode)
{
    z80_inst_outi(z80, opcode);
    if (z80->regfile.b == 0)
        return 16;
    z80->regfile.pc -= 2;
    return 21;
}

/*
    OUTD (0xEDAB):
    OUT (C), (HL); DEC HL; DEC B
*/
static uint8_t z80_inst_outd(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t hl = get_pair(z80, REG_HL);
    uint8_t *b = &z80->regfile.b;
    bool h = !!(((*b & 0x0F) - 1) & 0x10);

    write_port(z80, z80->regfile.c, mmu_read_byte(z80->mmu, hl));
    set_pair(z80, REG_HL, hl - 1);
    (*b)--;

    update_flags(z80, 0, 1, *b == 0x7F, !!(*b & 0x08), h, !!(*b & 0x20),
        *b == 0, !!(*b & 0x80), 0xFE);
    z80->regfile.pc++;
    return 16;
}

/*
    OTDR (0xEDBB):
    OUTD; JR NZ, -2
*/
static uint8_t z80_inst_otdr(Z80 *z80, uint8_t opcode)
{
    z80_inst_outd(z80, opcode);
    if (z80->regfile.b == 0)
        return 16;
    z80->regfile.pc -= 2;
    return 21;
}

/*
    NOP2:
    No operation is performed twice; i.e., 2 NOPs-worth of cycles are spent.
    Used for unimplemented extended and index instructions.
*/
static uint8_t z80_inst_nop2(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    z80->regfile.pc++;
    return 8;
}

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
