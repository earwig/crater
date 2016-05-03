/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#define POS(x) (!((x) & 0x80))
#define NEG(x)   ((x) & 0x80)

#define CARRY(lh, op, rh) (((lh) op (rh)) & 0x100)
#define HALF(lh, op, rh) ((((lh) & 0x0F) op ((rh) & 0x0F)) & 0x10)
#define ZERO(x) ((x) == 0)
#define SIGN(x) NEG(x)
#define SUB 1

#define CARRY2(lh, op, rh) (((lh) op (rh)) & 0x10000)

#define OV_ADD(lh, rh, res) \
    (POS(lh) && POS(rh) && NEG(res) || NEG(lh) && NEG(rh) && POS(res))
#define OV_SUB(lh, rh, res) \
    (POS(lh) && NEG(rh) && NEG(res) || NEG(lh) && POS(rh) && POS(res))

#define PARITY(x) (!(__builtin_popcount(x) % 2))

#define F3(x) ((x) & 0x08)
#define F5(x) ((x) & 0x20)

/*
    Set the flags for an 8-bit ADD or ADC instruction.
*/
static inline void set_flags_add8(Z80 *z80, uint16_t rh)
{
    uint8_t lh = z80->regs.a;
    uint8_t res = lh + rh;
    set_flags(z80, CARRY(lh, +, rh), !SUB, OV_ADD(lh, rh, res), F3(res),
        HALF(lh, +, rh), F5(res), ZERO(res), SIGN(res), 0xFF);
}

/*
    Set the flags for an 8-bit SUB or SBC instruction.
*/
static inline void set_flags_sub8(Z80 *z80, uint16_t rh)
{
    uint8_t lh = z80->regs.a;
    uint8_t res = lh - rh;
    set_flags(z80, CARRY(lh, -, rh), SUB, OV_SUB(lh, rh, res), F3(res),
        HALF(lh, -, rh), F5(res), ZERO(res), SIGN(res), 0xFF);
}

/*
    Set the flags for a CP instruction.
*/
static inline void set_flags_cp(Z80 *z80, uint16_t rh)
{
    uint8_t lh = z80->regs.a;
    uint8_t res = lh - rh;
    set_flags(z80, CARRY(lh, -, rh), SUB, OV_SUB(lh, rh, res), F3(rh),
        HALF(lh, -, rh), F5(rh), ZERO(res), SIGN(res), 0xFF);
}

/*
    Set the flags for an AND, XOR, or OR instruction.
*/
static inline void set_flags_bitwise(Z80 *z80, uint8_t res, bool is_and)
{
    set_flags(z80, 0, 0, PARITY(res), F3(res), is_and, F5(res), ZERO(res),
        SIGN(res), 0xFF);
}

/*
    Set the flags for an 8-bit INC instruction.
*/
static inline void set_flags_inc(Z80 *z80, uint8_t val)
{
    uint8_t res = val + 1;
    set_flags(z80, 0, !SUB, OV_ADD(val, 1, res), F3(res), HALF(val, +, 1),
        F5(res), ZERO(res), SIGN(res), 0xFE);
}

/*
    Set the flags for an 8-bit DEC instruction.
*/
static inline void set_flags_dec(Z80 *z80, uint8_t val)
{
    uint8_t res = val - 1;
    set_flags(z80, 0, SUB, OV_SUB(val, 1, res), F3(res), HALF(val, -, 1),
        F5(res), ZERO(res), SIGN(res), 0xFE);
}

/*
    Set the flags for a 16-bit ADD instruction.
*/
static inline void set_flags_add16(Z80 *z80, uint16_t lh, uint16_t rh)
{
    uint16_t res = lh + rh;
    set_flags(z80, CARRY2(lh, +, rh), !SUB, 0, F3(res >> 8),
        HALF(lh >> 8, +, rh >> 8), F5(res >> 8), 0, 0, 0x3B);
}

/*
    Set the flags for a 16-bit ADC instruction.
*/
static inline void set_flags_adc16(Z80 *z80, uint16_t lh, uint32_t rh)
{
    uint16_t res = lh + rh;
    set_flags(z80, CARRY2(lh, +, rh), !SUB, OV_ADD(lh >> 8, rh >> 8, res >> 8),
        F3(res >> 8), HALF(lh >> 8, +, rh >> 8), F5(res >> 8), ZERO(res),
        SIGN(res >> 8), 0xFF);
}

/*
    Set the flags for a 16-bit SBC instruction.
*/
static inline void set_flags_sbc16(Z80 *z80, uint16_t lh, uint32_t rh)
{
    uint16_t res = lh - rh;
    set_flags(z80, CARRY2(lh, -, rh), SUB, OV_SUB(lh >> 8, rh >> 8, res >> 8),
        F3(res >> 8), HALF(lh >> 8, -, rh >> 8), F5(res >> 8), ZERO(res),
        SIGN(res >> 8), 0xFF);
}

/*
    Set the flags for a BIT instruction.
*/
static inline void set_flags_bit(Z80 *z80, uint8_t val, uint8_t bit)
{
    bool z = ZERO((val >> bit) & 1);
    if (z)
        set_flags(z80, 0, 0, z, 0,        1, 0,        z, 0,        0xFE);
    else
        set_flags(z80, 0, 0, z, bit == 3, 1, bit == 5, z, bit == 7, 0xFE);
}

/*
    Set the flags for a RLCA/RLA/RRCA/RRA instruction.
*/
static inline void set_flags_bitrota(Z80 *z80, uint8_t bit)
{
    uint8_t a = z80->regs.a;
    set_flags(z80, bit, 0, 0, F3(a), 0, F5(a), 0, 0, 0x3B);
}

/*
    Set the flags for a RLC/RL/RRC/RR/SLA/SRA/SL1/SRL instruction.
*/
static inline void set_flags_bitshift(Z80 *z80, uint8_t res, uint8_t bit)
{
    set_flags(z80, bit, 0, PARITY(res), F3(res), 0, F5(res), ZERO(res),
        SIGN(res), 0xFF);
}

/*
    Set the flags for a DAA instruction.
*/
static inline void set_flags_daa(Z80 *z80, uint8_t old, uint8_t adjust)
{
    uint8_t res = z80->regs.a;

    bool c = adjust >= 0x60;
    bool h = get_flag(z80, FLAG_SUBTRACT) ?
        (get_flag(z80, FLAG_HALFCARRY) && (old & 0x0F) < 0x06) :
        ((old & 0x0F) > 0x09);

    set_flags(z80, c, 0, PARITY(res), F3(res), h, F5(res), ZERO(res),
        SIGN(res), 0xFD);
}

/*
    Set the flags for a CPL instruction.
*/
static inline void set_flags_cpl(Z80 *z80)
{
    uint8_t res = z80->regs.a;
    set_flags(z80, 0, 1, 0, F3(res), 1, F5(res), 0, 0, 0x3A);
}

/*
    Set the flags for a NEG instruction.
*/
static inline void set_flags_neg(Z80 *z80)
{
    uint8_t res = z80->regs.a;
    uint8_t val = -res;
    set_flags(z80, CARRY(0, -, val), SUB, OV_SUB(0, val, res), F3(res),
        HALF(0, -, val), F5(res), ZERO(res), SIGN(res), 0xFF);
}

/*
    Set the flags for a CCF instruction.
*/
static inline void set_flags_ccf(Z80 *z80)
{
    uint8_t val = z80->regs.a;
    bool c = get_flag(z80, FLAG_CARRY);
    set_flags(z80, 1 - c, 0, 0, F3(val), c, F5(val), 0, 0, 0x3B);
}

/*
    Set the flags for a SCF instruction.
*/
static inline void set_flags_scf(Z80 *z80)
{
    uint8_t val = z80->regs.a;
    set_flags(z80, 1, 0, 0, F3(val), 0, F5(val), 0, 0, 0x3B);
}

/*
    Set the flags for a LDI/LDIR/LDD/LDDR instruction.
*/
static inline void set_flags_blockxfer(Z80 *z80, uint8_t val)
{
    bool pv = z80->regs.bc != 0;
    uint8_t res = val + z80->regs.a;
    set_flags(z80, 0, 0, pv, res & 0x08, 0, res & 0x02, 0, 0, 0x3E);
}

/*
    Set the flags for an IN instruction.
*/
static inline void set_flags_in(Z80 *z80, uint8_t val)
{
    set_flags(z80, 0, 0, PARITY(val), F3(val), 0, F5(val), ZERO(val),
        SIGN(val), 0xFE);
}

/*
    Set the flags for an INI/INIR/IND/INDR/OUTI/OTIR/OUTD/OTDR instruction.
*/
static inline void set_flags_blockio(Z80 *z80)
{
    set_flags_dec(z80, z80->regs.b);
}

#undef POS
#undef NEG
#undef CARRY
#undef HALF
#undef ZERO
#undef SIGN
#undef SUB
#undef CARRY2
#undef OV_ADD
#undef OV_SUB
#undef PARITY
#undef F3
#undef F5
