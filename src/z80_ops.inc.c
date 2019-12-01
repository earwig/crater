/* Copyright (C) 2014-2019 Ben Kurtovic <ben.kurtovic@gmail.com>
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
    z80->regs.pc++;
    return 4;
}

/*
    LD r, n (0x06, 0x0E, 0x16, 0x1E, 0x26, 0x2E, 0x3E):
    Load n (8-bit immediate) into r (8-bit register).
*/
static uint8_t z80_inst_ld_r_n(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg = extract_reg(z80, opcode);
    *reg = mmu_read_byte(z80->mmu, ++z80->regs.pc);
    z80->regs.pc++;
    return 7;
}

/*
    LD r, (HL) (0x46, 0x4E, 0x56, 0x5E, 0x66, 0x6E, 0x7E):
    Load the memory pointed to by HL into r (8-bit register).
*/
static uint8_t z80_inst_ld_r_hl(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg = extract_reg(z80, opcode);
    *reg = mmu_read_byte(z80->mmu, z80->regs.hl);
    z80->regs.pc++;
    return 7;
}

/*
    LD r, (IXY+d) (0xDD46, 0xDD4E, 0xDD56, 0xDD5E, 0xDD66, 0xDD6E, 0xDD7E,
                   0xFD46, 0xFD4E, 0xFD56, 0xFD5E, 0xFD66, 0xFD6E, 0xFD7E):
    Load (IX+d) or (IY+d) into r (8-bit register).
*/
static uint8_t z80_inst_ld_r_ixy(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg = extract_reg(z80, opcode);
    uint16_t addr = get_index_addr(z80, ++z80->regs.pc);
    *reg = mmu_read_byte(z80->mmu, addr);
    z80->regs.pc++;
    return 19;
}

/*
    LD (HL), r (0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x77):
    Load r (8-bit register) into the memory pointed to by HL.
*/
static uint8_t z80_inst_ld_hl_r(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg = extract_reg(z80, opcode << 3);
    mmu_write_byte(z80->mmu, z80->regs.hl, *reg);
    z80->regs.pc++;
    return 7;
}

/*
    LD (IXY+d), r (0xDD70, 0xDD71, 0xDD72, 0xDD73, 0xDD74, 0xDD75, 0xDD77,
                   0xFD70, 0xFD71, 0xFD72, 0xFD73, 0xFD74, 0xFD75, 0xFD77):
    Load r (8-bit register) into (IX+d) or (IY+d).
*/
static uint8_t z80_inst_ld_ixy_r(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg = extract_reg(z80, opcode << 3);
    uint16_t addr = get_index_addr(z80, ++z80->regs.pc);
    mmu_write_byte(z80->mmu, addr, *reg);
    z80->regs.pc++;
    return 19;
}

/*
    LD (HL), n (0x36):
    Load n (8-bit immediate) into the memory address pointed to by HL.
*/
static uint8_t z80_inst_ld_hl_n(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t byte = mmu_read_byte(z80->mmu, ++z80->regs.pc);
    mmu_write_byte(z80->mmu, z80->regs.hl, byte);
    z80->regs.pc++;
    return 10;
}

/*
    LD (IXY+d), n (0xDD36, 0xFD36):
    Load n (8-bit immediate) into (IX+d) or (IY+d).
*/
static uint8_t z80_inst_ld_ixy_n(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t addr = get_index_addr(z80, ++z80->regs.pc);
    uint8_t byte = mmu_read_byte(z80->mmu, ++z80->regs.pc);
    mmu_write_byte(z80->mmu, addr, byte);
    z80->regs.pc++;
    return 19;
}

/*
    LD A, (BC/DE) (0x0A, 0x1A):
    Load the memory pointed to BC or DE into A.
*/
static uint8_t z80_inst_ld_a_bcde(Z80 *z80, uint8_t opcode)
{
    uint16_t addr = *extract_pair(z80, opcode);
    z80->regs.a = mmu_read_byte(z80->mmu, addr);
    z80->regs.pc++;
    return 7;
}

/*
    LD A, (nn) (0x3A):
    Load memory at address nn into A.
*/
static uint8_t z80_inst_ld_a_nn(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t addr = mmu_read_double(z80->mmu, ++z80->regs.pc);
    z80->regs.a = mmu_read_byte(z80->mmu, addr);
    z80->regs.pc += 2;
    return 13;
}

/*
    LD (BC/DE), A (0x02, 0x12):
    Load A into the memory address pointed to by BC or DE.
*/
static uint8_t z80_inst_ld_bcde_a(Z80 *z80, uint8_t opcode)
{
    uint16_t addr = *extract_pair(z80, opcode);
    mmu_write_byte(z80->mmu, addr, z80->regs.a);
    z80->regs.pc++;
    return 7;
}

/*
    LD (nn), A (0x32):
    Load A into memory address nn.
*/
static uint8_t z80_inst_ld_nn_a(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t addr = mmu_read_double(z80->mmu, ++z80->regs.pc);
    mmu_write_byte(z80->mmu, addr, z80->regs.a);
    z80->regs.pc += 2;
    return 13;
}

/*
    LD A, I (0xED57):
    Load I into A.
*/
static uint8_t z80_inst_ld_a_i(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    z80->regs.a = z80->regs.i;
    set_flags_ld_a_ir(z80);
    z80->regs.pc++;
    return 9;
}

/*
    LD A, R (0xED5F):
    Load R into A.
*/
static uint8_t z80_inst_ld_a_r(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    z80->regs.a = z80->regs.r;
    set_flags_ld_a_ir(z80);
    z80->regs.pc++;
    return 9;
}

/*
    LD I, A (0xED47):
    Load A into I.
*/
static uint8_t z80_inst_ld_i_a(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    z80->regs.i = z80->regs.a;
    z80->regs.pc++;
    return 9;
}

/*
    LD R, A (0xED4F):
    Load A into R.
*/
static uint8_t z80_inst_ld_r_a(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    z80->regs.r = z80->regs.a;
    z80->regs.pc++;
    return 9;
}

/*
    LD dd, nn (0x01, 0x11, 0x21, 0x31):
    Load nn (16-bit immediate) into dd (16-bit register).
*/
static uint8_t z80_inst_ld_dd_nn(Z80 *z80, uint8_t opcode)
{
    *extract_pair(z80, opcode) = mmu_read_double(z80->mmu, ++z80->regs.pc);
    z80->regs.pc += 2;
    return 10;
}

/*
    LD IXY, nn (0xDD21, 0xFD21):
    Load nn (16-bit immediate) into IX or IY.
*/
static uint8_t z80_inst_ld_ixy_nn(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    *z80->regs.ixy = mmu_read_double(z80->mmu, ++z80->regs.pc);
    z80->regs.pc += 2;
    return 14;
}

/*
    LD HL, (nn) (0x2A):
    Load memory at address nn into HL.
*/
static uint8_t z80_inst_ld_hl_inn(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t addr = mmu_read_double(z80->mmu, ++z80->regs.pc);
    z80->regs.hl = mmu_read_double(z80->mmu, addr);
    z80->regs.pc += 2;
    return 16;
}

/*
    LD dd, (nn) (0xED4B, 0xED5B, 0xED6B, 0xED7B):
    Load memory at address nn into dd (16-bit register).
*/
static uint8_t z80_inst_ld_dd_inn(Z80 *z80, uint8_t opcode)
{
    uint16_t addr = mmu_read_double(z80->mmu, ++z80->regs.pc);
    *extract_pair(z80, opcode) = mmu_read_double(z80->mmu, addr);
    z80->regs.pc += 2;
    return 20;
}

/*
    LD IXY, (nn) (0xDD2A, 0xFD2A):
    Load memory at address nn into IX or IY.
*/
static uint8_t z80_inst_ld_ixy_inn(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t addr = mmu_read_double(z80->mmu, ++z80->regs.pc);
    *z80->regs.ixy = mmu_read_double(z80->mmu, addr);
    z80->regs.pc += 2;
    return 20;
}

/*
    LD (nn), HL: (0x22):
    Load HL into memory address nn.
*/
static uint8_t z80_inst_ld_inn_hl(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t addr = mmu_read_double(z80->mmu, ++z80->regs.pc);
    mmu_write_double(z80->mmu, addr, z80->regs.hl);
    z80->regs.pc += 2;
    return 16;
}

/*
    LD (nn), dd (0xED43, 0xED53, 0xED63, 0xED73);
    Load dd (16-bit register) into memory address nn.
*/
static uint8_t z80_inst_ld_inn_dd(Z80 *z80, uint8_t opcode)
{
    uint16_t addr = mmu_read_double(z80->mmu, ++z80->regs.pc);
    mmu_write_double(z80->mmu, addr, *extract_pair(z80, opcode));
    z80->regs.pc += 2;
    return 20;
}

/*
    LD (nn), IXY (0xDD22, 0xFD22):
    Load IX or IY into memory address nn.
*/
static uint8_t z80_inst_ld_inn_ixy(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t addr = mmu_read_double(z80->mmu, ++z80->regs.pc);
    mmu_write_double(z80->mmu, addr, *z80->regs.ixy);
    z80->regs.pc += 2;
    return 20;
}

/*
    LD SP, HL (0xF9):
    Load HL into SP.
*/
static uint8_t z80_inst_ld_sp_hl(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    z80->regs.sp = z80->regs.hl;
    z80->regs.pc++;
    return 6;
}

/*
    LD SP, IXY (0xDDF9, 0xFDF9):
    Load IX or IY into SP.
*/
static uint8_t z80_inst_ld_sp_ixy(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    *z80->regs.ixy = z80->regs.hl;
    z80->regs.pc++;
    return 10;
}

/*
    PUSH qq (0xC5, 0xD5, 0xE5, 0xF5):
    Push qq onto the stack, and decrement SP by two.
*/
static uint8_t z80_inst_push_qq(Z80 *z80, uint8_t opcode)
{
    stack_push(z80, *extract_pair_qq(z80, opcode));
    z80->regs.pc++;
    return 11;
}

/*
    PUSH IXY (0xDDE5, 0xFDE5):
    Push IX or IY onto the stack, and decrement SP by two.
*/
static uint8_t z80_inst_push_ixy(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    stack_push(z80, *z80->regs.ixy);
    z80->regs.pc++;
    return 15;
}

/*
    POP qq (0xC1, 0xD1, 0xE1, 0xF1):
    Pop qq from the stack, and increment SP by two.
*/
static uint8_t z80_inst_pop_qq(Z80 *z80, uint8_t opcode)
{
    *extract_pair_qq(z80, opcode) = stack_pop(z80);
    z80->regs.pc++;
    return 10;
}

/*
    POP IXY (0xDDE1, 0xFDE1):
    Pop IX or IY from the stack, and increment SP by two.
*/
static uint8_t z80_inst_pop_ixy(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    *z80->regs.ixy = stack_pop(z80);
    z80->regs.pc++;
    return 14;
}

/*
    EX DE, HL (0xEB):
    Exchange DE with HL.
*/
static uint8_t z80_inst_ex_de_hl(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t temp = z80->regs.de;
    z80->regs.de = z80->regs.hl;
    z80->regs.hl = temp;
    z80->regs.pc++;
    return 4;
}

/*
    EX AF, AF' (0x08):
    Exchange AF with AF'.
*/
static uint8_t z80_inst_ex_af_af(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t temp = z80->regs.af;
    z80->regs.af = z80->regs.af_;
    z80->regs.af_ = temp;
    z80->regs.pc++;
    return 4;
}

/*
    EXX (0xD9):
    Exchange the 16-bit registers with their shadows
    (BC <=> BC', DE <=> DE', HL <=> HL').
*/
static uint8_t z80_inst_exx(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t bc = z80->regs.bc, de = z80->regs.de, hl = z80->regs.hl;
    z80->regs.bc = z80->regs.bc_;
    z80->regs.de = z80->regs.de_;
    z80->regs.hl = z80->regs.hl_;
    z80->regs.bc_ = bc;
    z80->regs.de_ = de;
    z80->regs.hl_ = hl;
    z80->regs.pc++;
    return 4;
}

/*
    EX (SP), HL (0xE3):
    Exchange the memory pointed to by SP with HL.
*/
static uint8_t z80_inst_ex_sp_hl(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t hl = z80->regs.hl, sp = z80->regs.sp;
    z80->regs.hl = mmu_read_double(z80->mmu, sp);
    mmu_write_double(z80->mmu, sp, hl);
    z80->regs.pc++;
    return 19;
}

/*
    EX (SP), IXY (0xDDE3, 0xFDE3):
    Exchange the memory pointed to by SP with IX or IY.
*/
static uint8_t z80_inst_ex_sp_ixy(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t ixy = *z80->regs.ixy, sp = z80->regs.sp;
    *z80->regs.ixy = mmu_read_double(z80->mmu, sp);
    mmu_write_double(z80->mmu, sp, ixy);
    z80->regs.pc++;
    return 23;
}

/*
    LDI (0xEDA0):
    LD (DE), (HL); INC HL; INC DE; DEC BC;
*/
static uint8_t z80_inst_ldi(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t value = mmu_read_byte(z80->mmu, z80->regs.hl);
    mmu_write_byte(z80->mmu, z80->regs.de, value);

    z80->regs.hl++;
    z80->regs.de++;
    z80->regs.bc--;

    set_flags_blockxfer(z80, value);
    z80->regs.pc++;
    return 16;
}

/*
    LDIR (0xEDB0):
    LDI; JR PV, -2
*/
static uint8_t z80_inst_ldir(Z80 *z80, uint8_t opcode)
{
    z80_inst_ldi(z80, opcode);
    if (z80->regs.bc == 0)
        return 16;
    z80->regs.pc -= 2;
    return 21;
}

/*
    LDD (0xEDA8):
    LD (DE), (HL); DEC HL; DEC DE; DEC BC;
*/
static uint8_t z80_inst_ldd(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t value = mmu_read_byte(z80->mmu, z80->regs.hl);
    mmu_write_byte(z80->mmu, z80->regs.de, value);

    z80->regs.hl--;
    z80->regs.de--;
    z80->regs.bc--;

    set_flags_blockxfer(z80, value);
    z80->regs.pc++;
    return 16;
}

/*
    LDDR (0xEDB8):
    LDD; JR PV, -2
*/
static uint8_t z80_inst_lddr(Z80 *z80, uint8_t opcode)
{
    z80_inst_ldd(z80, opcode);
    if (z80->regs.bc == 0)
        return 16;
    z80->regs.pc -= 2;
    return 21;
}

/*
    CPI (0xEDA1):
    CP (HL); INC HL; DEC BL
*/
static uint8_t z80_inst_cpi(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t value = mmu_read_byte(z80->mmu, z80->regs.hl);

    z80->regs.hl++;
    z80->regs.bc--;

    set_flags_blockcp(z80, value);
    z80->regs.pc++;
    return 16;
}

/*
    CPIR (0xEDB1):
    CPI; JR PV; -2
*/
static uint8_t z80_inst_cpir(Z80 *z80, uint8_t opcode)
{
    z80_inst_cpi(z80, opcode);
    if (z80->regs.bc == 0)
        return 16;
    z80->regs.pc -= 2;
    return 21;
}

/*
    CPD (0xEDA9):
    CP (HL); DEC HL; DEC BL
*/
static uint8_t z80_inst_cpd(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t value = mmu_read_byte(z80->mmu, z80->regs.hl);

    z80->regs.hl--;
    z80->regs.bc--;

    set_flags_blockcp(z80, value);
    z80->regs.pc++;
    return 16;
}

/*
    CPDR (0xEDB9):
    CPD; JR PV; -2
*/
static uint8_t z80_inst_cpdr(Z80 *z80, uint8_t opcode)
{
    z80_inst_cpd(z80, opcode);
    if (z80->regs.bc == 0)
        return 16;
    z80->regs.pc -= 2;
    return 21;
}

/*
    ADD A, r (0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x87):
    Add r (8-bit register) to A.
*/
static uint8_t z80_inst_add_a_r(Z80 *z80, uint8_t opcode)
{
    uint8_t value = *extract_reg(z80, opcode << 3);

    set_flags_add8(z80, value);
    z80->regs.a += value;
    z80->regs.pc++;
    return 4;
}

/*
    ADD A, n (0xC6):
    Add n (8-bit immediate) to A.
*/
static uint8_t z80_inst_add_a_n(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t value = mmu_read_byte(z80->mmu, ++z80->regs.pc);

    set_flags_add8(z80, value);
    z80->regs.a += value;
    z80->regs.pc++;
    return 7;
}

/*
    ADD A, (HL) (0x86):
    Add (HL) to A.
*/
static uint8_t z80_inst_add_a_hl(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t value = mmu_read_byte(z80->mmu, z80->regs.hl);

    set_flags_add8(z80, value);
    z80->regs.a += value;
    z80->regs.pc++;
    return 7;
}

/*
    ADD A, (IXY+d) (0xDD86, 0xFD86):
    Add (IX+d) or (IY+d) to A.
*/
static uint8_t z80_inst_add_a_ixy(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t addr = get_index_addr(z80, ++z80->regs.pc);
    uint8_t value = mmu_read_byte(z80->mmu, addr);

    set_flags_add8(z80, value);
    z80->regs.a += value;
    z80->regs.pc++;
    return 19;
}

/*
    ADC A, r (0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8F):
    Add r (8-bit register) plus the carry flag to A.
*/
static uint8_t z80_inst_adc_a_r(Z80 *z80, uint8_t opcode)
{
    uint16_t value = *extract_reg(z80, opcode << 3);
    value += get_flag(z80, FLAG_CARRY);

    set_flags_add8(z80, value);
    z80->regs.a += value;
    z80->regs.pc++;
    return 4;
}

/*
    ADC A, n (0xCE):
    Add n (8-bit immediate) plus the carry flag to A.
*/
static uint8_t z80_inst_adc_a_n(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t value = mmu_read_byte(z80->mmu, ++z80->regs.pc);
    value += get_flag(z80, FLAG_CARRY);

    set_flags_add8(z80, value);
    z80->regs.a += value;
    z80->regs.pc++;
    return 7;
}

/*
    ADC A, (HL) (0x8E):
    Add (HL) plus the carry flag to A.
*/
static uint8_t z80_inst_adc_a_hl(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t value = mmu_read_byte(z80->mmu, z80->regs.hl);
    value += get_flag(z80, FLAG_CARRY);

    set_flags_add8(z80, value);
    z80->regs.a += value;
    z80->regs.pc++;
    return 7;
}

/*
    ADC A, (IXY+d) (0xDD8E, 0xFD8E):
    Add (IX+d) or (IY+d) plus the carry flag to A.
*/
static uint8_t z80_inst_adc_a_ixy(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t addr = get_index_addr(z80, ++z80->regs.pc);
    uint16_t value = mmu_read_byte(z80->mmu, addr);
    value += get_flag(z80, FLAG_CARRY);

    set_flags_add8(z80, value);
    z80->regs.a += value;
    z80->regs.pc++;
    return 19;
}

/*
    SUB r (0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x97):
    Subtract r (8-bit register) from A.
*/
static uint8_t z80_inst_sub_r(Z80 *z80, uint8_t opcode)
{
    uint8_t value = *extract_reg(z80, opcode << 3);

    set_flags_sub8(z80, value);
    z80->regs.a -= value;
    z80->regs.pc++;
    return 4;
}

/*
    SUB n (0xD6):
    Subtract n (8-bit immediate) from A.
*/
static uint8_t z80_inst_sub_n(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t value = mmu_read_byte(z80->mmu, ++z80->regs.pc);

    set_flags_sub8(z80, value);
    z80->regs.a -= value;
    z80->regs.pc++;
    return 7;
}

/*
    SUB (HL)
    Subtract (HL) from A.
*/
static uint8_t z80_inst_sub_hl(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t value = mmu_read_byte(z80->mmu, z80->regs.hl);

    set_flags_sub8(z80, value);
    z80->regs.a -= value;
    z80->regs.pc++;
    return 7;
}

/*
    SUB (IXY+d)
    Subtract (IX+d) or (IY+d) from A.
*/
static uint8_t z80_inst_sub_ixy(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t addr = get_index_addr(z80, ++z80->regs.pc);
    uint8_t value = mmu_read_byte(z80->mmu, addr);

    set_flags_sub8(z80, value);
    z80->regs.a -= value;
    z80->regs.pc++;
    return 19;
}

/*
    SBC A, r (0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9F):
    Subtract r (8-bit register) plus the carry flag from A.
*/
static uint8_t z80_inst_sbc_a_r(Z80 *z80, uint8_t opcode)
{
    uint8_t value = *extract_reg(z80, opcode << 3);
    value += get_flag(z80, FLAG_CARRY);

    set_flags_sub8(z80, value);
    z80->regs.a -= value;
    z80->regs.pc++;
    return 4;
}

/*
    SBC A, n (0xDE):
    Subtract n (8-bit immediate) plus the carry flag from A.
*/
static uint8_t z80_inst_sbc_a_n(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t value = mmu_read_byte(z80->mmu, ++z80->regs.pc);
    value += get_flag(z80, FLAG_CARRY);

    set_flags_sub8(z80, value);
    z80->regs.a -= value;
    z80->regs.pc++;
    return 7;
}

/*
    SBC A, (HL) (0x9E):
    Subtract (HL) plus the carry flag from A.
*/
static uint8_t z80_inst_sbc_a_hl(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t value = mmu_read_byte(z80->mmu, z80->regs.hl);
    value += get_flag(z80, FLAG_CARRY);

    set_flags_sub8(z80, value);
    z80->regs.a -= value;
    z80->regs.pc++;
    return 7;
}

/*
    SBC A, (IXY+d) (0xDD9E, 0xFD9E):
    Subtract (IX+d) or (IY+d) plus the carry flag from A.
*/
static uint8_t z80_inst_sbc_a_ixy(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t addr = get_index_addr(z80, ++z80->regs.pc);
    uint8_t value = mmu_read_byte(z80->mmu, addr);
    value += get_flag(z80, FLAG_CARRY);

    set_flags_sub8(z80, value);
    z80->regs.a -= value;
    z80->regs.pc++;
    return 19;
}

/*
    AND r (0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA7):
    Bitwise AND A with r (8-bit register).
*/
static uint8_t z80_inst_and_r(Z80 *z80, uint8_t opcode)
{
    uint8_t value = *extract_reg(z80, opcode << 3);
    uint8_t res = z80->regs.a &= value;

    set_flags_bitwise(z80, res, true);
    z80->regs.pc++;
    return 4;
}

/*
    AND n (0xE6):
    Bitwise AND A with n (8-bit immediate).
*/
static uint8_t z80_inst_and_n(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t value = mmu_read_byte(z80->mmu, ++z80->regs.pc);
    uint8_t res = z80->regs.a &= value;

    set_flags_bitwise(z80, res, true);
    z80->regs.pc++;
    return 7;
}

/*
    AND (HL) (0xA6):
    Bitwise AND A with (HL).
*/
static uint8_t z80_inst_and_hl(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t value = mmu_read_byte(z80->mmu, z80->regs.hl);
    uint8_t res = z80->regs.a &= value;

    set_flags_bitwise(z80, res, true);
    z80->regs.pc++;
    return 7;
}

/*
    AND (IXY+d) (0xDDA6, 0xFDA6):
    Bitwise AND A with (IX+d) or (IY+d).
*/
static uint8_t z80_inst_and_ixy(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t addr = get_index_addr(z80, ++z80->regs.pc);
    uint8_t value = mmu_read_byte(z80->mmu, addr);
    uint8_t res = z80->regs.a &= value;

    set_flags_bitwise(z80, res, true);
    z80->regs.pc++;
    return 19;
}

/*
    OR r (0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB7):
    Bitwise OR A with r (8-bit register).
*/
static uint8_t z80_inst_or_r(Z80 *z80, uint8_t opcode)
{
    uint8_t value = *extract_reg(z80, opcode << 3);
    uint8_t res = z80->regs.a |= value;

    set_flags_bitwise(z80, res, false);
    z80->regs.pc++;
    return 4;
}

/*
    OR n (0xF6):
    Bitwise OR A with n (8-bit immediate).
*/
static uint8_t z80_inst_or_n(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t value = mmu_read_byte(z80->mmu, ++z80->regs.pc);
    uint8_t res = z80->regs.a |= value;

    set_flags_bitwise(z80, res, false);
    z80->regs.pc++;
    return 7;
}

/*
    OR (HL) (0xB6):
    Bitwise OR A with (HL).
*/
static uint8_t z80_inst_or_hl(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t value = mmu_read_byte(z80->mmu, z80->regs.hl);
    uint8_t res = z80->regs.a |= value;

    set_flags_bitwise(z80, res, false);
    z80->regs.pc++;
    return 7;
}

/*
    OR (IXY+d) (0xDDB6, 0xFDB6):
    Bitwise OR A with (IX+d) or (IY+d).
*/
static uint8_t z80_inst_or_ixy(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t addr = get_index_addr(z80, ++z80->regs.pc);
    uint8_t value = mmu_read_byte(z80->mmu, addr);
    uint8_t res = z80->regs.a |= value;

    set_flags_bitwise(z80, res, false);
    z80->regs.pc++;
    return 19;
}

/*
    XOR r (0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAF):
    Bitwise XOR A with r (8-bit register).
*/
static uint8_t z80_inst_xor_r(Z80 *z80, uint8_t opcode)
{
    uint8_t value = *extract_reg(z80, opcode << 3);
    uint8_t res = z80->regs.a ^= value;

    set_flags_bitwise(z80, res, false);
    z80->regs.pc++;
    return 4;
}

/*
    XOR n (0xEE):
    Bitwise XOR A with n (8-bit immediate).
*/
static uint8_t z80_inst_xor_n(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t value = mmu_read_byte(z80->mmu, ++z80->regs.pc);
    uint8_t res = z80->regs.a ^= value;

    set_flags_bitwise(z80, res, false);
    z80->regs.pc++;
    return 7;
}

/*
    XOR (HL) (0xAE):
    Bitwise XOR A with (HL).
*/
static uint8_t z80_inst_xor_hl(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t value = mmu_read_byte(z80->mmu, z80->regs.hl);
    uint8_t res = z80->regs.a ^= value;

    set_flags_bitwise(z80, res, false);
    z80->regs.pc++;
    return 7;
}

/*
    XOR (IXY+d) (0xDDAE, 0xFDAE):
    Bitwise XOR A with (IX+d) or (IY+d).
*/
static uint8_t z80_inst_xor_ixy(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t addr = get_index_addr(z80, ++z80->regs.pc);
    uint8_t value = mmu_read_byte(z80->mmu, addr);
    uint8_t res = z80->regs.a ^= value;

    set_flags_bitwise(z80, res, false);
    z80->regs.pc++;
    return 19;
}

/*
    CP r (0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBF):
    Set flags as if r (8-bit register) had been subtracted from A.
*/
static uint8_t z80_inst_cp_r(Z80 *z80, uint8_t opcode)
{
    uint8_t value = *extract_reg(z80, opcode << 3);

    set_flags_cp(z80, value);
    z80->regs.pc++;
    return 4;
}

/*
    CP n (0xFE):
    Set flags as if n (8-bit immediate) had been subtracted from A.
*/
static uint8_t z80_inst_cp_n(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t value = mmu_read_byte(z80->mmu, ++z80->regs.pc);

    set_flags_cp(z80, value);
    z80->regs.pc++;
    return 7;
}

/*
    CP (HL) (0xBE):
    Set flags as if (HL) had been subtracted from A.
*/
static uint8_t z80_inst_cp_hl(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t value = mmu_read_byte(z80->mmu, z80->regs.hl);

    set_flags_cp(z80, value);
    z80->regs.pc++;
    return 7;
}

/*
    CP (IXY+d) (0xDDBE, 0xFDBE):
    Set flags as if (IX+d) or (IY+d) had been subtracted from A.
*/
static uint8_t z80_inst_cp_ixy(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t addr = get_index_addr(z80, ++z80->regs.pc);
    uint8_t value = mmu_read_byte(z80->mmu, addr);

    set_flags_cp(z80, value);
    z80->regs.pc++;
    return 19;
}

/*
    INC r (0x04, 0x0C, 0x14, 0x1C, 0x24, 0x2C, 0x3C):
    Increment r (8-bit register).
*/
static uint8_t z80_inst_inc_r(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg = extract_reg(z80, opcode);

    set_flags_inc(z80, *reg);
    (*reg)++;
    z80->regs.pc++;
    return 4;
}

/*
    INC (HL) (0x34):
    Increment the memory address pointed to by HL.
*/
static uint8_t z80_inst_inc_hl(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t byte = mmu_read_byte(z80->mmu, z80->regs.hl);

    set_flags_inc(z80, byte);
    mmu_write_byte(z80->mmu, z80->regs.hl, ++byte);
    z80->regs.pc++;
    return 11;
}

/*
    INC (IXY+d) (0xDD34, 0xFD34):
    Increment (IX+d) or (IY+d).
*/
static uint8_t z80_inst_inc_ixy(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t addr = get_index_addr(z80, ++z80->regs.pc);
    uint8_t byte = mmu_read_byte(z80->mmu, addr);

    set_flags_inc(z80, byte);
    mmu_write_byte(z80->mmu, addr, ++byte);
    z80->regs.pc++;
    return 23;
}

/*
    DEC r (0x05, 0x0D, 0x15, 0x1D, 0x25, 0x2D, 0x3D):
    Decrement r (8-bit register).
*/
static uint8_t z80_inst_dec_r(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg = extract_reg(z80, opcode);

    set_flags_dec(z80, *reg);
    (*reg)--;
    z80->regs.pc++;
    return 4;
}

/*
    DEC (HL) (0x35):
    Decrement the memory address pointed to by HL.
*/
static uint8_t z80_inst_dec_hl(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t byte = mmu_read_byte(z80->mmu, z80->regs.hl);

    set_flags_dec(z80, byte);
    mmu_write_byte(z80->mmu, z80->regs.hl, --byte);
    z80->regs.pc++;
    return 11;
}

/*
    DEC (IXY+d) (0xDD35, 0xFD35):
    Decrement (IX+d) or (IY+d).
*/
static uint8_t z80_inst_dec_ixy(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint16_t addr = get_index_addr(z80, ++z80->regs.pc);
    uint8_t byte = mmu_read_byte(z80->mmu, addr);

    set_flags_dec(z80, byte);
    mmu_write_byte(z80->mmu, addr, --byte);
    z80->regs.pc++;
    return 23;
}

/*
    DAA (0x27):
    Adjust A for BCD addition and subtraction.
*/
static uint8_t z80_inst_daa(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t a = z80->regs.a, adjust = 0x00;
    bool n = get_flag(z80, FLAG_SUBTRACT);

    if ((a & 0x0F) > 0x09 || get_flag(z80, FLAG_HALFCARRY))
        adjust += 0x06;

    uint8_t temp = n ? (a - adjust) : (a + adjust);
    if ((temp >> 4) > 0x09 || get_flag(z80, FLAG_CARRY))
        adjust += 0x60;

    z80->regs.a += n ? -adjust : adjust;
    set_flags_daa(z80, a, adjust);
    z80->regs.pc++;
    return 4;
}

/*
    CPL (0x2F):
    Invert A.
*/
static uint8_t z80_inst_cpl(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    z80->regs.a = ~z80->regs.a;
    set_flags_cpl(z80);
    z80->regs.pc++;
    return 4;
}

/*
    NEG (0xED44, 0xED4C, 0xED54, 0xED5C, 0xED64, 0xED6C, 0xED74, 0xED7C):
    Negate A.
*/
static uint8_t z80_inst_neg(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    z80->regs.a = -z80->regs.a;
    set_flags_neg(z80);
    z80->regs.pc++;
    return 8;
}

/*
    CCF (0x3F):
    Invert the carry flag.
*/
static uint8_t z80_inst_ccf(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    set_flags_ccf(z80);
    z80->regs.pc++;
    return 4;
}

/*
    SCF (0x37):
    Set the carry flag.
*/
static uint8_t z80_inst_scf(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    set_flags_scf(z80);
    z80->regs.pc++;
    return 4;
}

/*
    NOP (0x00):
    No operation is performed.
*/
static uint8_t z80_inst_nop(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    z80->regs.pc++;
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
    z80->regs.iff1 = false;
    z80->regs.iff2 = false;
    z80->regs.pc++;
    return 4;
}

/*
    EI (0xFB):
    Enable maskable interrupts by setting both flip-flops.
*/
static uint8_t z80_inst_ei(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    z80->regs.iff1 = true;
    z80->regs.iff2 = true;
    z80->irq_wait = true;
    z80->regs.pc++;
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
            z80->regs.im_a = false;  // Interrupt mode 0
            z80->regs.im_b = false;
            break;
        case 0x56:
        case 0x76:
            z80->regs.im_a = true;  // Interrupt mode 1
            z80->regs.im_b = false;
            break;
        case 0x5E:
        case 0x7E:
            z80->regs.im_a = true;  // Interrupt mode 2
            z80->regs.im_b = true;
            break;
    }

    z80->regs.pc++;
    return 8;
}

/*
    ADD HL, ss (0x09, 0x19, 0x29, 0x39):
    Add ss to HL.
*/
static uint8_t z80_inst_add_hl_ss(Z80 *z80, uint8_t opcode)
{
    uint16_t lh = z80->regs.hl, rh = *extract_pair(z80, opcode);
    z80->regs.hl += rh;

    set_flags_add16(z80, lh, rh);
    z80->regs.pc++;
    return 11;
}

/*
    ADC HL, ss (0xED4A, 0xED5A, 0xED6A, 0xED7A):
    Add ss plus the carry flag to HL.
*/
static uint8_t z80_inst_adc_hl_ss(Z80 *z80, uint8_t opcode)
{
    uint16_t lh = z80->regs.hl;
    uint32_t rh = *extract_pair(z80, opcode) + get_flag(z80, FLAG_CARRY);
    z80->regs.hl += rh;

    set_flags_adc16(z80, lh, rh);
    z80->regs.pc++;
    return 15;
}

/*
    SBC HL, ss (0xED42, 0xED52, 0xED62, 0xED72):
    Subtract ss with carry from HL.
*/
static uint8_t z80_inst_sbc_hl_ss(Z80 *z80, uint8_t opcode)
{
    uint16_t lh = z80->regs.hl;
    uint32_t rh = *extract_pair(z80, opcode) + get_flag(z80, FLAG_CARRY);
    z80->regs.hl -= rh;

    set_flags_sbc16(z80, lh, rh);
    z80->regs.pc++;
    return 15;
}

/*
    ADD IXY, pp (0xDD09, 0xDD19, 0xDD29, 0xDD39, 0xFD09, 0xFD19, 0xFD29,
        0xFD39):
    Add pp to IX or IY.
*/
static uint8_t z80_inst_add_ixy_pp(Z80 *z80, uint8_t opcode)
{
    uint16_t lh = *z80->regs.ixy, rh = *extract_pair_pp(z80, opcode);
    *z80->regs.ixy += rh;

    set_flags_add16(z80, lh, rh);
    z80->regs.pc++;
    return 15;
}

/*
    INC ss (0x03, 0x13, 0x23, 0x33):
    Increment ss (16-bit register).
*/
static uint8_t z80_inst_inc_ss(Z80 *z80, uint8_t opcode)
{
    (*extract_pair(z80, opcode))++;
    z80->regs.pc++;
    return 6;
}

/*
    INC IXY (0xDD23, 0xFD23):
    Increment IX or IY.
*/
static uint8_t z80_inst_inc_xy(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    (*z80->regs.ixy)++;
    z80->regs.pc++;
    return 10;
}

/*
    DEC ss (0x0B, 0x1B, 0x2B, 0x3B):
    Decrement ss (16-bit register).
*/
static uint8_t z80_inst_dec_ss(Z80 *z80, uint8_t opcode)
{
    (*extract_pair(z80, opcode))--;
    z80->regs.pc++;
    return 6;
}

/*
    DEC IXY (0xDD2B, 0xFD2B):
    Decrement IX or IY.
*/
static uint8_t z80_inst_dec_xy(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    (*z80->regs.ixy)--;
    z80->regs.pc++;
    return 10;
}

/*
    RLCA (0x07):
    Rotate A left one bit. Bit 7 is copied to bit 0 and the carry flag.
*/
static uint8_t z80_inst_rlca(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t bit = (z80->regs.a & 0x80) >> 7;
    z80->regs.a <<= 1;
    z80->regs.a |= bit;

    set_flags_bitrota(z80, bit);
    z80->regs.pc++;
    return 4;
}

/*
    RLA (0x17):
    Rotate A left one bit. Carry flag is copied to bit 0, and bit 7 is copied
    to the carry flag.
*/
static uint8_t z80_inst_rla(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t carry = get_flag(z80, FLAG_CARRY);
    uint8_t bit = (z80->regs.a & 0x80) >> 7;
    z80->regs.a <<= 1;
    z80->regs.a |= carry;

    set_flags_bitrota(z80, bit);
    z80->regs.pc++;
    return 4;
}

/*
    RRCA (0x0F):
    Rotate A right one bit. Bit 0 is copied to bit 7 and the carry flag.
*/
static uint8_t z80_inst_rrca(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t bit = z80->regs.a & 0x01;
    z80->regs.a >>= 1;
    z80->regs.a |= (bit << 7);

    set_flags_bitrota(z80, bit);
    z80->regs.pc++;
    return 4;
}

/*
    RRA (0x1F):
    Rotate A right one bit. Carry flag is copied to bit 7, and bit 0 is copied
    to the carry flag.
*/
static uint8_t z80_inst_rra(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t carry = get_flag(z80, FLAG_CARRY);
    uint8_t bit = z80->regs.a & 0x01;
    z80->regs.a >>= 1;
    z80->regs.a |= (carry << 7);

    set_flags_bitrota(z80, bit);
    z80->regs.pc++;
    return 4;
}

/*
    RLC r (0xCB00, 0xCB01, 0xCB02, 0xCB03, 0xCB04, 0xCB05, 0xCB07):
    Rotate r left one bit. Bit 7 is copied to bit 0 and the carry flag.
*/
static uint8_t z80_inst_rlc_r(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg = extract_reg(z80, opcode << 3);
    uint8_t bit = ((*reg) & 0x80) >> 7;
    (*reg) <<= 1;
    (*reg) |= bit;

    set_flags_bitshift(z80, *reg, bit);
    z80->regs.pc++;
    return 8;
}

/*
    RLC (HL) (0xCB06):
    Rotate (HL) left one bit. Bit 7 is copied to bit 0 and the carry flag.
*/
static uint8_t z80_inst_rlc_hl(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t val = mmu_read_byte(z80->mmu, z80->regs.hl);
    uint8_t bit = (val & 0x80) >> 7;
    val <<= 1;
    val |= bit;

    mmu_write_byte(z80->mmu, z80->regs.hl, val);
    set_flags_bitshift(z80, val, bit);
    z80->regs.pc++;
    return 15;
}

/*
    RLC (IXY+d)
    TODO
*/

/*
    RL r (0xCB10, 0xCB11, 0xCB12, 0xCB13, 0xCB14, 0xCB15, 0xCB17):
    Rotate r left one bit. Carry flag is copied to bit 0, and bit 7 is copied
    to the carry flag.
*/
static uint8_t z80_inst_rl_r(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg = extract_reg(z80, opcode << 3);
    uint8_t carry = get_flag(z80, FLAG_CARRY);
    uint8_t bit = ((*reg) & 0x80) >> 7;
    (*reg) <<= 1;
    (*reg) |= carry;

    set_flags_bitshift(z80, *reg, bit);
    z80->regs.pc++;
    return 8;
}

/*
    RL (HL) (0xCB16):
    Rotate (HL) left one bit. Carry flag is copied to bit 0, and bit 7 is
    copied to the carry flag.
*/
static uint8_t z80_inst_rl_hl(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t val = mmu_read_byte(z80->mmu, z80->regs.hl);
    uint8_t carry = get_flag(z80, FLAG_CARRY);
    uint8_t bit = (val & 0x80) >> 7;
    val <<= 1;
    val |= carry;

    mmu_write_byte(z80->mmu, z80->regs.hl, val);
    set_flags_bitshift(z80, val, bit);
    z80->regs.pc++;
    return 15;
}

/*
    RL (IXY+d)
    TODO
*/

/*
    RRC r (0xCB08, 0xCB09, 0xCB0A, 0xCB0B, 0xCB0C, 0xCB0D, 0xCB0F):
    Rotate r right one bit. Bit 0 is copied to bit 7 and the carry flag.
*/
static uint8_t z80_inst_rrc_r(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg = extract_reg(z80, opcode << 3);
    uint8_t bit = (*reg) & 0x01;
    (*reg) >>= 1;
    (*reg) |= (bit << 7);

    set_flags_bitshift(z80, *reg, bit);
    z80->regs.pc++;
    return 8;
}

/*
    RRC (HL) (0xCB0E):
    Rotate (HL) right one bit. Bit 0 is copied to bit 7 and the carry flag.
*/
static uint8_t z80_inst_rrc_hl(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t val = mmu_read_byte(z80->mmu, z80->regs.hl);
    uint8_t bit = (val) & 0x01;
    val >>= 1;
    val |= (bit << 7);

    mmu_write_byte(z80->mmu, z80->regs.hl, val);
    set_flags_bitshift(z80, val, bit);
    z80->regs.pc++;
    return 15;
}

/*
    RRC (IXY+d)
    TODO
*/

/*
    RR r (0xCB18, 0xCB19, 0xCB1A, 0xCB1B, 0xCB1C, 0xCB1D, 0xCB1F):
    Rotate r right one bit. Carry flag is copied to bit 7, and bit 0 is copied
    to the carry flag.
*/
static uint8_t z80_inst_rr_r(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg = extract_reg(z80, opcode << 3);
    uint8_t carry = get_flag(z80, FLAG_CARRY);
    uint8_t bit = (*reg) & 0x01;
    (*reg) >>= 1;
    (*reg) |= (carry << 7);

    set_flags_bitshift(z80, *reg, bit);
    z80->regs.pc++;
    return 8;
}

/*
    RR (HL) (0xCB1E):
    Rotate (HL) right one bit. Carry flag is copied to bit 7, and bit 0 is
    copied to the carry flag.
*/
static uint8_t z80_inst_rr_hl(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t val = mmu_read_byte(z80->mmu, z80->regs.hl);
    uint8_t carry = get_flag(z80, FLAG_CARRY);
    uint8_t bit = (val) & 0x01;
    val >>= 1;
    val |= (carry << 7);

    mmu_write_byte(z80->mmu, z80->regs.hl, val);
    set_flags_bitshift(z80, val, bit);
    z80->regs.pc++;
    return 15;
}

/*
    RR (IXY+d)
    TODO
*/

/*
    SLA r (0xCB20, 0xCB21, 0xCB22, 0xCB23, 0xCB24, 0xCB25, 0xCB27):
    Shift r left one bit. 0 is copied to bit 0, and bit 7 is copied to the
    carry flag.
*/
static uint8_t z80_inst_sla_r(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg = extract_reg(z80, opcode << 3);
    uint8_t msb = ((*reg) & 0x80) >> 7;
    (*reg) <<= 1;

    set_flags_bitshift(z80, *reg, msb);
    z80->regs.pc++;
    return 8;
}

/*
    SLA (HL) (0xCB26):
    Shift (HL) left one bit. 0 is copied to bit 0, and bit 7 is copied to the
    carry flag.
*/
static uint8_t z80_inst_sla_hl(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t val = mmu_read_byte(z80->mmu, z80->regs.hl);
    uint8_t msb = (val & 0x80) >> 7;
    val <<= 1;

    mmu_write_byte(z80->mmu, z80->regs.hl, val);
    set_flags_bitshift(z80, val, msb);
    z80->regs.pc++;
    return 15;
}

/*
    SLA (IXY+d)
    TODO
*/

/*
    SRA r (0xCB28, 0xCB29, 0xCB2A, 0xCB2B, 0xCB2C, 0xCB2D, 0xCB2F):
    Arithmetic shift r right one bit. The previous bit 7 is copied to the new
    bit 7, and bit 0 is copied to the carry flag.
*/
static uint8_t z80_inst_sra_r(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg = extract_reg(z80, opcode << 3);
    uint8_t msb = (*reg) & 0x80, lsb = (*reg) & 0x01;
    (*reg) >>= 1;
    (*reg) |= msb;

    set_flags_bitshift(z80, *reg, lsb);
    z80->regs.pc++;
    return 8;
}

/*
    SRA (HL) (0xCB2E):
    Arithmetic shift (HL) right one bit. The previous bit 7 is copied to the
    new bit 7, and bit 0 is copied to the carry flag.
*/
static uint8_t z80_inst_sra_hl(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t val = mmu_read_byte(z80->mmu, z80->regs.hl);
    uint8_t msb = val & 0x80, lsb = val & 0x01;
    val >>= 1;
    val |= msb;

    mmu_write_byte(z80->mmu, z80->regs.hl, val);
    set_flags_bitshift(z80, val, lsb);
    z80->regs.pc++;
    return 8;
}

/*
    SRA (IXY+d)
    TODO
*/

/*
    SL1 r (0xCB30, 0xCB31, 0xCB32, 0xCB33, 0xCB34, 0xCB35, 0xCB37):
    Shift r left one bit. 1 is copied to bit 0, and bit 7 is copied to the
    carry flag.
*/
static uint8_t z80_inst_sl1_r(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg = extract_reg(z80, opcode << 3);
    uint8_t msb = ((*reg) & 0x80) >> 7;
    (*reg) <<= 1;
    (*reg) |= 1;

    set_flags_bitshift(z80, *reg, msb);
    z80->regs.pc++;
    return 8;
}

/*
    SL1 (HL) (0xCB36):
    Shift (HL) left one bit. 1 is copied to bit 0, and bit 7 is copied to the
    carry flag.
*/
static uint8_t z80_inst_sl1_hl(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t val = mmu_read_byte(z80->mmu, z80->regs.hl);
    uint8_t msb = (val & 0x80) >> 7;
    val <<= 1;
    val |= 1;

    mmu_write_byte(z80->mmu, z80->regs.hl, val);
    set_flags_bitshift(z80, val, msb);
    z80->regs.pc++;
    return 15;
}

/*
    SL1 (IXY+d)
    TODO
*/

/*
    SRL r (0xCB38, 0xCB39, 0xCB3A, 0xCB3B, 0xCB3C, 0xCB3D, 0xCB3F):
    Logical shift r right one bit. 0 is copied to bit 7, and bit 0 is copied to
    the carry flag.
*/
static uint8_t z80_inst_srl_r(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg = extract_reg(z80, opcode << 3);
    uint8_t lsb = (*reg) & 0x01;
    (*reg) >>= 1;

    set_flags_bitshift(z80, *reg, lsb);
    z80->regs.pc++;
    return 8;
}

/*
    SRL (HL) (0xCB3E):
    Logical shift (HL) right one bit. 0 is copied to bit 7, and bit 0 is copied
    to the carry flag.
*/
static uint8_t z80_inst_srl_hl(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t val = mmu_read_byte(z80->mmu, z80->regs.hl);
    uint8_t lsb = val & 0x01;
    val >>= 1;

    mmu_write_byte(z80->mmu, z80->regs.hl, val);
    set_flags_bitshift(z80, val, lsb);
    z80->regs.pc++;
    return 8;
}

/*
    SRL (IXY+d)
    TODO
*/

/*
    RLD (0xED6F):
    Low nibble of (HL) is copied to high nibble of (HL). Old high nibble of
    (HL) is copied to low nibble of A. Old low nibble of A is copied to low
    nibble of (HL).
*/
static uint8_t z80_inst_rld(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t hl = mmu_read_byte(z80->mmu, z80->regs.hl);
    uint8_t newhl = (hl << 4) | (z80->regs.a & 0x0F);
    z80->regs.a = (z80->regs.a & 0xF0) | (hl >> 4);
    mmu_write_byte(z80->mmu, z80->regs.hl, newhl);
    set_flags_rd(z80);
    return 18;
}

/*
    RRD (0xED67):
    Low nibble of A is copied to high nibble of (HL). Old high nibble of
    (HL) is copied to low nibble of (HL). Old low nibble of (HL) is copied to
    low nibble of A.
*/
static uint8_t z80_inst_rrd(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t hl = mmu_read_byte(z80->mmu, z80->regs.hl);
    uint8_t newhl = (z80->regs.a << 4) | (hl >> 4);
    z80->regs.a = (z80->regs.a & 0xF0) | (hl & 0x0F);
    mmu_write_byte(z80->mmu, z80->regs.hl, newhl);
    set_flags_rd(z80);
    return 18;
}

/*
    BIT b, r   (0xCB40, 0xCB41, 0xCB42, 0xCB43, 0xCB44, 0xCB45, 0xCB47, 0xCB48,
        0xCB49, 0xCB4A, 0xCB4B, 0xCB4C, 0xCB4D, 0xCB4F, 0xCB50, 0xCB51, 0xCB52,
        0xCB53, 0xCB54, 0xCB55, 0xCB57, 0xCB58, 0xCB59, 0xCB5A, 0xCB5B, 0xCB5C,
        0xCB5D, 0xCB5F, 0xCB60, 0xCB61, 0xCB62, 0xCB63, 0xCB64, 0xCB65, 0xCB67,
        0xCB68, 0xCB69, 0xCB6A, 0xCB6B, 0xCB6C, 0xCB6D, 0xCB6F, 0xCB70, 0xCB71,
        0xCB72, 0xCB73, 0xCB74, 0xCB75, 0xCB77, 0xCB78, 0xCB79, 0xCB7A, 0xCB7B,
        0xCB7C, 0xCB7D, 0xCB7F):
    Test bit b of r (8-bit register).
*/
static uint8_t z80_inst_bit_b_r(Z80 *z80, uint8_t opcode)
{
    uint8_t val = *extract_reg(z80, opcode << 3);
    uint8_t bit = (opcode >> 3) & 0x07;

    set_flags_bit(z80, val, bit);
    z80->regs.pc++;
    return 8;
}

/*
    BIT b, (HL) (0xCB46, 0xCB4E, 0xCB56, 0xCB5E, 0xCB66, 0xCB6E, 0xCB76,
        0xCB7E):
    Test bit b of (HL).
*/
static uint8_t z80_inst_bit_b_hl(Z80 *z80, uint8_t opcode)
{
    uint8_t val = mmu_read_byte(z80->mmu, z80->regs.hl);
    uint8_t bit = (opcode >> 3) & 0x07;

    set_flags_bit(z80, val, bit);
    z80->regs.pc++;
    return 12;
}

/*
    BIT b, (IXY+d) (0xDDCB40-0xDDCB7F, 0xFDCB40-0xFDCB7F):
    Test bit b of (IX+d) or (IY+d).
*/
static uint8_t z80_inst_bit_b_ixy(Z80 *z80, uint8_t opcode)
{
    uint16_t addr = get_index_addr(z80, z80->regs.pc - 1);
    uint8_t val = mmu_read_byte(z80->mmu, addr);
    uint8_t bit = (opcode >> 3) & 0x07;

    set_flags_bit(z80, val, bit);
    z80->regs.pc++;
    return 20;
}

/*
    SET b, r   (0xCBC0, 0xCBC1, 0xCBC2, 0xCBC3, 0xCBC4, 0xCBC5, 0xCBC7, 0xCBC8,
        0xCBC9, 0xCBCA, 0xCBCB, 0xCBCC, 0xCBCD, 0xCBCF, 0xCBD0, 0xCBD1, 0xCBD2,
        0xCBD3, 0xCBD4, 0xCBD5, 0xCBD7, 0xCBD8, 0xCBD9, 0xCBDA, 0xCBDB, 0xCBDC,
        0xCBDD, 0xCBDF, 0xCBE0, 0xCBE1, 0xCBE2, 0xCBE3, 0xCBE4, 0xCBE5, 0xCBE7,
        0xCBE8, 0xCBE9, 0xCBEA, 0xCBEB, 0xCBEC, 0xCBED, 0xCBEF, 0xCBF0, 0xCBF1,
        0xCBF2, 0xCBF3, 0xCBF4, 0xCBF5, 0xCBF7, 0xCBF8, 0xCBF9, 0xCBFA, 0xCBFB,
        0xCBFC, 0xCBFD, 0xCBFF):
    Set bit b of r.
*/
static uint8_t z80_inst_set_b_r(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg = extract_reg(z80, opcode << 3);
    uint8_t bit = (opcode >> 3) & 0x07;
    *reg |= 1 << bit;
    z80->regs.pc++;
    return 8;
}

/*
    SET b, (HL) (0xCBC6, 0xCBCE, 0xCBD6, 0xCBDE, 0xCBE6, 0xCBEE, 0xCBF6,
        0xCBFE):
    Reset bit b of (HL).
*/
static uint8_t z80_inst_set_b_hl(Z80 *z80, uint8_t opcode)
{
    uint8_t val = mmu_read_byte(z80->mmu, z80->regs.hl);
    uint8_t bit = (opcode >> 3) & 0x07;
    val |= 1 << bit;
    mmu_write_byte(z80->mmu, z80->regs.hl, val);
    z80->regs.pc++;
    return 15;
}

/*
    SET b, (IXY+d) (0xDDCBC6, 0xDDCBCE, 0xDDCBD6, 0xDDCBDE, 0xDDCBE6, 0xDDCBEE,
          0xDDCBF6, 0xDDCBFE, 0xFDCBC6, 0xFDCBCE, 0xFDCBD6, 0xFDCBDE, 0xFDCBE6,
          0xFDCBEE, 0xFDCBF6, 0xFDCBFE):
    Set bit b of (IX+d) or (IY+d).
*/
static uint8_t z80_inst_set_b_ixy(Z80 *z80, uint8_t opcode)
{
    uint16_t addr = get_index_addr(z80, z80->regs.pc - 1);
    uint8_t val = mmu_read_byte(z80->mmu, addr);
    uint8_t bit = (opcode >> 3) & 0x07;
    val |= 1 << bit;
    mmu_write_byte(z80->mmu, addr, val);
    z80->regs.pc++;
    return 15;
}

/*
    RES b, r   (0xCB80, 0xCB81, 0xCB82, 0xCB83, 0xCB84, 0xCB85, 0xCB87, 0xCB88,
        0xCB89, 0xCB8A, 0xCB8B, 0xCB8C, 0xCB8D, 0xCB8F, 0xCB90, 0xCB91, 0xCB92,
        0xCB93, 0xCB94, 0xCB95, 0xCB97, 0xCB98, 0xCB99, 0xCB9A, 0xCB9B, 0xCB9C,
        0xCB9D, 0xCB9F, 0xCBA0, 0xCBA1, 0xCBA2, 0xCBA3, 0xCBA4, 0xCBA5, 0xCBA7,
        0xCBA8, 0xCBA9, 0xCBAA, 0xCBAB, 0xCBAC, 0xCBAD, 0xCBAF, 0xCBB0, 0xCBB1,
        0xCBB2, 0xCBB3, 0xCBB4, 0xCBB5, 0xCBB7, 0xCBB8, 0xCBB9, 0xCBBA, 0xCBBB,
        0xCBBC, 0xCBBD, 0xCBBF):
    Reset bit b of r.
*/
static uint8_t z80_inst_res_b_r(Z80 *z80, uint8_t opcode)
{
    uint8_t *reg = extract_reg(z80, opcode << 3);
    uint8_t bit = (opcode >> 3) & 0x07;
    *reg &= ~(1 << bit);
    z80->regs.pc++;
    return 8;
}

/*
    RES b, (HL) (0xCB86, 0xCB8E, 0xCB96, 0xCB9E, 0xCBA6, 0xCBAE, 0xCBB6,
        0xCBBE):
    Reset bit b of (HL).
*/
static uint8_t z80_inst_res_b_hl(Z80 *z80, uint8_t opcode)
{
    uint8_t val = mmu_read_byte(z80->mmu, z80->regs.hl);
    uint8_t bit = (opcode >> 3) & 0x07;
    val &= ~(1 << bit);
    mmu_write_byte(z80->mmu, z80->regs.hl, val);
    z80->regs.pc++;
    return 15;
}

/*
    RES b, (IXY+d) (0xDDCB86, 0xDDCB8E, 0xDDCB96, 0xDDCB9E, 0xDDCBA6, 0xDDCBAE,
          0xDDCBB6, 0xDDCBBE, 0xFDCB86, 0xFDCB8E, 0xFDCBA6, 0xFDCBAE, 0xFDCBB6,
          0xFDCBBE, 0xFDCBC6, 0xFDCBCE):
    Set bit b of (IX+d) or (IY+d).
*/
static uint8_t z80_inst_res_b_ixy(Z80 *z80, uint8_t opcode)
{
    uint16_t addr = get_index_addr(z80, z80->regs.pc - 1);
    uint8_t val = mmu_read_byte(z80->mmu, addr);
    uint8_t bit = (opcode >> 3) & 0x07;
    val &= ~(1 << bit);
    mmu_write_byte(z80->mmu, addr, val);
    z80->regs.pc++;
    return 15;
}

/*
    JP nn (0xC3):
    Jump to nn (16-bit immediate).
*/
static uint8_t z80_inst_jp_nn(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    z80->regs.pc = mmu_read_double(z80->mmu, ++z80->regs.pc);
    return 10;
}

/*
    JP cc, nn (0xC2, 0xCA, 0xD2, 0xDA, 0xE2, 0xEA, 0xF2, 0xFA):
    Jump to nn (16-bit immediate) if cc (condition) is true.
*/
static uint8_t z80_inst_jp_cc_nn(Z80 *z80, uint8_t opcode)
{
    if (extract_cond(z80, opcode))
        z80->regs.pc = mmu_read_double(z80->mmu, ++z80->regs.pc);
    else
        z80->regs.pc += 3;
    return 10;
}

/*
    JR e (0x18):
    Relative jump e (signed 8-bit immediate) bytes.
*/
static uint8_t z80_inst_jr_e(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    int8_t jump = mmu_read_byte(z80->mmu, z80->regs.pc + 1);
    z80->regs.pc += jump + 2;
    return 12;
}


/*
    JR cc, e (0x20, 0x28, 0x30, 0x38):
    Relative jump e (signed 8-bit immediate) bytes if cc (condition) is true.
*/
static uint8_t z80_inst_jr_cc_e(Z80 *z80, uint8_t opcode)
{
    if (extract_cond(z80, opcode - 0x20)) {
        int8_t jump = mmu_read_byte(z80->mmu, z80->regs.pc + 1);
        z80->regs.pc += jump + 2;
        return 12;
    } else {
        z80->regs.pc += 2;
        return 7;
    }
}

/*
    JP (HL) (0xE9):
    Jump to HL (*NOT* the memory pointed to by HL).
*/
static uint8_t z80_inst_jp_hl(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    z80->regs.pc = z80->regs.hl;
    return 4;
}

/*
    JP (IXY) (0xDDE9, 0xFDE9):
    Jump to IX or IY.
*/
static uint8_t z80_inst_jp_ixy(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    z80->regs.pc = *z80->regs.ixy;
    return 8;
}

/*
    DJNZ, e (0x10):
    Decrement B and relative jump e bytes (signed 8-bit immediate) if non-zero.
*/
static uint8_t z80_inst_djnz_e(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    z80->regs.b--;
    if (z80->regs.b != 0) {
        int8_t jump = mmu_read_byte(z80->mmu, z80->regs.pc + 1);
        z80->regs.pc += jump + 2;
        return 13;
    } else {
        z80->regs.pc += 2;
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
    stack_push(z80, z80->regs.pc + 3);
    z80->regs.pc = mmu_read_double(z80->mmu, ++z80->regs.pc);
    return 17;
}

/*
    CALL cc, nn (0xC4, 0xCC, 0xD4, 0xDC, 0xE4, 0xEC, 0xF4, 0xFC):
    Push PC+3 onto the stack and jump to nn (16-bit immediate) if cc is true.
*/
static uint8_t z80_inst_call_cc_nn(Z80 *z80, uint8_t opcode)
{
    if (extract_cond(z80, opcode)) {
        stack_push(z80, z80->regs.pc + 3);
        z80->regs.pc = mmu_read_double(z80->mmu, ++z80->regs.pc);
        return 17;
    } else {
        z80->regs.pc += 3;
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
    z80->regs.pc = stack_pop(z80);
    return 10;
}

/*
    RET cc (0xC0, 0xC8, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8):
    Pop PC from the stack if cc is true.
*/
static uint8_t z80_inst_ret_cc(Z80 *z80, uint8_t opcode)
{
    if (extract_cond(z80, opcode)) {
        z80->regs.pc = stack_pop(z80);
        return 11;
    } else {
        z80->regs.pc++;
        return 5;
    }
}

/*
    RETI (0xED4D):
    Pop PC from the stack.
*/
static uint8_t z80_inst_reti(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    z80->regs.pc = stack_pop(z80);
    return 14;
}

/*
    RETN (0xED45, 0xED55, 0xED5D, 0xED65, 0xED6D, 0xED75, 0xED7D):
    Pop PC from the stack, and copy to IFF2 to IFF1.
*/
static uint8_t z80_inst_retn(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    z80->regs.pc = stack_pop(z80);
    z80->regs.iff1 = z80->regs.iff2;
    return 14;
}

/*
    RST p (0xC7, 0xCF, 0xD7, 0xDF, 0xE7, 0xEF, 0xF7, 0xFF):
    Push PC+1 onto the stack and jump to p (opcode & 0x38).
*/
static uint8_t z80_inst_rst_p(Z80 *z80, uint8_t opcode)
{
    stack_push(z80, z80->regs.pc + 1);
    z80->regs.pc = opcode & 0x38;
    return 11;
}

/*
    IN A, (n) (0xDB):
    Read a byte from port n into A.
*/
static uint8_t z80_inst_in_a_n(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t port = mmu_read_byte(z80->mmu, ++z80->regs.pc);
    z80->regs.a = io_port_read(z80->io, port);
    z80->regs.pc++;
    return 11;
}

/*
    IN r, (C) (0xED40, 0xED48, 0xED50, 0xED58, 0xED60, 0xED68, 0xED70, 0xED78):
    Read a byte from port C into r, or affect flags only if 0xED70.
*/
static uint8_t z80_inst_in_r_c(Z80 *z80, uint8_t opcode)
{
    uint8_t data = io_port_read(z80->io, z80->regs.c);
    if (opcode != 0x70)
        *extract_reg(z80, opcode) = data;

    set_flags_in(z80, data);
    z80->regs.pc++;
    return 12;
}

/*
    INI (0xEDA2):
    IN (HL), (C); INC HL; DEC B
*/
static uint8_t z80_inst_ini(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t data = io_port_read(z80->io, z80->regs.c);
    mmu_write_byte(z80->mmu, z80->regs.hl, data);
    set_flags_blockio(z80);

    z80->regs.hl++;
    z80->regs.b--;
    z80->regs.pc++;
    return 16;
}

/*
    INIR (0xEDB2):
    INI; JR NZ, -2
*/
static uint8_t z80_inst_inir(Z80 *z80, uint8_t opcode)
{
    z80_inst_ini(z80, opcode);
    if (z80->regs.b == 0)
        return 16;
    z80->regs.pc -= 2;
    return 21;
}

/*
    IND (0xEDAA):
    IN (HL), (C); DEC HL; DEC B
*/
static uint8_t z80_inst_ind(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t data = io_port_read(z80->io, z80->regs.c);
    mmu_write_byte(z80->mmu, z80->regs.hl, data);
    set_flags_blockio(z80);

    z80->regs.hl--;
    z80->regs.b--;
    z80->regs.pc++;
    return 16;
}

/*
    INDR (0xEDBA):
    IND; JR NZ, -2
*/
static uint8_t z80_inst_indr(Z80 *z80, uint8_t opcode)
{
    z80_inst_ind(z80, opcode);
    if (z80->regs.b == 0)
        return 16;
    z80->regs.pc -= 2;
    return 21;
}

/*
    OUT (n), A (0xD3):
    Write a byte from A into port n.
*/
static uint8_t z80_inst_out_n_a(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t port = mmu_read_byte(z80->mmu, ++z80->regs.pc);
    io_port_write(z80->io, port, z80->regs.a);
    z80->regs.pc++;
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
    io_port_write(z80->io, z80->regs.c, value);
    z80->regs.pc++;
    return 12;
}

/*
    OUTI (0xEDA3):
    OUT (C), (HL); INC HL; DEC B
*/
static uint8_t z80_inst_outi(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t data = mmu_read_byte(z80->mmu, z80->regs.hl);
    io_port_write(z80->io, z80->regs.c, data);
    set_flags_blockio(z80);

    z80->regs.hl++;
    z80->regs.b--;
    z80->regs.pc++;
    return 16;
}

/*
    OTIR (0xEDB3):
    OUTI; JR NZ, -2
*/
static uint8_t z80_inst_otir(Z80 *z80, uint8_t opcode)
{
    z80_inst_outi(z80, opcode);
    if (z80->regs.b == 0)
        return 16;
    z80->regs.pc -= 2;
    return 21;
}

/*
    OUTD (0xEDAB):
    OUT (C), (HL); DEC HL; DEC B
*/
static uint8_t z80_inst_outd(Z80 *z80, uint8_t opcode)
{
    (void) opcode;
    uint8_t data = mmu_read_byte(z80->mmu, z80->regs.hl);
    io_port_write(z80->io, z80->regs.c, data);
    set_flags_blockio(z80);

    z80->regs.hl--;
    z80->regs.b--;
    z80->regs.pc++;
    return 16;
}

/*
    OTDR (0xEDBB):
    OUTD; JR NZ, -2
*/
static uint8_t z80_inst_otdr(Z80 *z80, uint8_t opcode)
{
    z80_inst_outd(z80, opcode);
    if (z80->regs.b == 0)
        return 16;
    z80->regs.pc -= 2;
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
    z80->regs.pc++;
    return 8;
}

/*
    0xED:
    Handle an extended instruction.
*/
static uint8_t z80_prefix_extended(Z80 *z80, uint8_t opcode)
{
    opcode = mmu_read_byte(z80->mmu, ++z80->regs.pc);
    return (*instruction_table_extended[opcode])(z80, opcode);
}

/*
    0xED:
    Handle a bit instruction.
*/
static uint8_t z80_prefix_bits(Z80 *z80, uint8_t opcode)
{
    opcode = mmu_read_byte(z80->mmu, ++z80->regs.pc);
    return (*instruction_table_bits[opcode])(z80, opcode);
}

/*
    0xDD, 0xFD:
    Handle an index instruction.
*/
static uint8_t z80_prefix_index(Z80 *z80, uint8_t opcode)
{
    if (opcode == 0xDD) {
        z80->regs.ixy = &z80->regs.ix;
        z80->regs.ih  = &z80->regs.ixh;
        z80->regs.il  = &z80->regs.ixl;
    } else {
        z80->regs.ixy = &z80->regs.iy;
        z80->regs.ih  = &z80->regs.iyh;
        z80->regs.il  = &z80->regs.iyl;
    }

    opcode = mmu_read_byte(z80->mmu, ++z80->regs.pc);
    return (*instruction_table_index[opcode])(z80, opcode);
}

/*
    0xDDCB, 0xFDCB:
    Handle an index-bit instruction.
*/
static uint8_t z80_prefix_index_bits(Z80 *z80, uint8_t opcode)
{
    opcode = mmu_read_byte(z80->mmu, z80->regs.pc += 2);
    return (*instruction_table_index_bits[opcode])(z80, opcode);
}

#include "z80_tables.inc.c"
