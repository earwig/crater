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
