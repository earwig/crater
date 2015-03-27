/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include "logging.h"
#include "z80.h"

#define REG_AF 0
#define REG_BC 1
#define REG_DE 2
#define REG_HL 3

#define FLAG_CARRY     0
#define FLAG_SUBTRACT  1
#define FLAG_PARITY    2
#define FLAG_OVERFLOW  2
#define FLAG_UNDOC_3   3
#define FLAG_HALFCARRY 4
#define FLAG_UNDOC_5   5
#define FLAG_ZERO      6
#define FLAG_SIGN      7

/*
    Initialize a Z80 object.

    Register values are invalid until z80_power() is called. No other Z80
    functions should be called before it.
*/
void z80_init(Z80 *z80, MMU *mmu)
{
    z80->mmu = mmu;
    z80->except = true;
    z80->exc_code = Z80_EXC_NOT_POWERED;
    z80->exc_data = 0;
}

/*
    Power on the Z80, setting registers to their default values.

    This also clears the exception flag, which is necessary before the Z80 can
    begin emulation.
*/
void z80_power(Z80 *z80)
{
    Z80RegFile *regfile = &z80->regfile;

    regfile->a  = regfile->f  = 0xFF;
    regfile->b  = regfile->c  = 0xFF;
    regfile->d  = regfile->e  = 0xFF;
    regfile->h  = regfile->l  = 0xFF;
    regfile->a_ = regfile->f_ = 0xFF;
    regfile->b_ = regfile->c_ = 0xFF;
    regfile->d_ = regfile->e_ = 0xFF;
    regfile->h_ = regfile->l_ = 0xFF;

    regfile->ix = 0xFFFF;
    regfile->iy = 0xFFFF;
    regfile->sp = 0xFFFF;
    regfile->pc = 0x0000;

    regfile->i = 0xFF;
    regfile->r = 0xFF;

    regfile->im_a = regfile->im_b = 0;
    regfile->iff1 = regfile->iff2 = 0;

    z80->except = false;
    z80->pending_cycles = 0;
}

/*
    Get the value of a register pair.
*/
static inline uint16_t get_pair(Z80 *z80, uint8_t pair)
{
    Z80RegFile *regfile = &z80->regfile;

    switch (pair) {
        case REG_AF: return (((uint16_t) regfile->a) << 8) + regfile->f;
        case REG_BC: return (((uint16_t) regfile->b) << 8) + regfile->c;
        case REG_DE: return (((uint16_t) regfile->d) << 8) + regfile->e;
        case REG_HL: return (((uint16_t) regfile->h) << 8) + regfile->l;
        default: FATAL("Invalid call: get_pair(z80, %u)", pair)
    }
}

/*
    Set the value of a register pair.
*/
static inline void set_pair(Z80 *z80, uint8_t pair, uint16_t value)
{
    Z80RegFile *regfile = &z80->regfile;

    switch (pair) {
        case REG_AF: regfile->a = value >> 8; regfile->f = value; break;
        case REG_BC: regfile->b = value >> 8; regfile->c = value; break;
        case REG_DE: regfile->d = value >> 8; regfile->e = value; break;
        case REG_HL: regfile->h = value >> 8; regfile->l = value; break;
        default: FATAL("Invalid call: set_pair(z80, %u, 0x%04X)", pair, value)
    }
}

/*
    Return whether a particular flag is set in the F register.
*/
static inline bool get_flag(const Z80 *z80, uint8_t flag)
{
    return z80->regfile.f & (1 << flag);
}

/*
    Return whether a particular flag is set in the F' register.
*/
static inline bool get_shadow_flag(const Z80 *z80, uint8_t flag)
{
    return z80->regfile.f_ & (1 << flag);
}

/*
    Update the F register flags according to the set bits in the mask.
*/
static inline void update_flags(Z80 *z80, bool c, bool n, bool pv, bool f3,
                                bool h, bool f5, bool z, bool s, uint8_t mask)
{
    z80->regfile.f = (~mask & z80->regfile.f) | (mask & (
        c  << FLAG_CARRY     |
        n  << FLAG_SUBTRACT  |
        pv << FLAG_PARITY    |
        f3 << FLAG_UNDOC_3   |
        h  << FLAG_HALFCARRY |
        f5 << FLAG_UNDOC_5   |
        z  << FLAG_ZERO      |
        s  << FLAG_SIGN));
}

/*
    Return the CPU's current interrupt mode.
*/
static inline uint8_t get_interrupt_mode(const Z80 *z80)
{
    if (!z80->regfile.im_a)
        return 0;
    if (!z80->regfile.im_b)
        return 1;
    return 2;
}

/*
    Increment the refresh counter register, R.
*/
static inline void increment_refresh_counter(Z80 *z80)
{
    z80->regfile.r = (z80->regfile.r & 0x80) | ((z80->regfile.r + 1) & 0x7F);
}

#include "z80_instructions.inc"

/*
    Emulate the given number of cycles of the Z80, or until an exception.

    The return value indicates whether the exception flag is set. If it is,
    then emulation must be stopped because further calls to z80_do_cycles()
    will have no effect. The exception flag can be reset with z80_power().
*/
bool z80_do_cycles(Z80 *z80, double cycles)
{
    cycles -= z80->pending_cycles;
    while (cycles > 0 && !z80->except) {
        uint8_t opcode = mmu_read_byte(z80->mmu, z80->regfile.pc);
        increment_refresh_counter(z80);
        cycles -= (*instruction_lookup_table[opcode])(z80, opcode) - 2;
    }

    z80->pending_cycles = -cycles;
    return z80->except;
}

#ifdef DEBUG_MODE
/*
    DEBUG FUNCTION: Print out all register values to stdout.
*/
void z80_dump_registers(const Z80 *z80)
{
    const Z80RegFile *regfile = &z80->regfile;
    DEBUG("Dumping Z80 register values:")

    DEBUG("- AF:   0x%02X%02X (C: %u, N: %u, P/V: %u, H: %u, Z: %u, S: %u)",
          regfile->a, regfile->f,
          get_flag(z80, FLAG_CARRY),
          get_flag(z80, FLAG_SUBTRACT),
          get_flag(z80, FLAG_PARITY),
          get_flag(z80, FLAG_HALFCARRY),
          get_flag(z80, FLAG_ZERO),
          get_flag(z80, FLAG_SIGN))

    DEBUG("- BC:   0x%02X%02X", regfile->b, regfile->c)
    DEBUG("- DE:   0x%02X%02X", regfile->d, regfile->e)
    DEBUG("- HL:   0x%02X%02X", regfile->h, regfile->l)

    DEBUG("- AF':  0x%02X%02X (C: %u, N: %u, P/V: %u, H: %u, Z: %u, S: %u)",
          regfile->a_, regfile->f_,
          get_shadow_flag(z80, FLAG_CARRY),
          get_shadow_flag(z80, FLAG_SUBTRACT),
          get_shadow_flag(z80, FLAG_PARITY),
          get_shadow_flag(z80, FLAG_HALFCARRY),
          get_shadow_flag(z80, FLAG_ZERO),
          get_shadow_flag(z80, FLAG_SIGN))

    DEBUG("- BC':  0x%02X%02X", regfile->b_, regfile->c_)
    DEBUG("- DE':  0x%02X%02X", regfile->d_, regfile->e_)
    DEBUG("- HL':  0x%02X%02X", regfile->h_, regfile->l_)

    DEBUG("- IX:   0x%04X", regfile->ix)
    DEBUG("- IY:   0x%04X", regfile->iy)
    DEBUG("- SP:   0x%04X", regfile->sp)
    DEBUG("- PC:   0x%04X", regfile->pc)

    DEBUG("- I:    0x%2X", regfile->i)
    DEBUG("- R:    0x%2X", regfile->r)

    DEBUG("- IM:   0b%u%u (mode: %u)", regfile->im_a, regfile->im_b,
          get_interrupt_mode(z80))
    DEBUG("- IFF1: %u", regfile->iff1)
    DEBUG("- IFF2: %u", regfile->iff2)
}
#endif
