/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include "logging.h"
#include "z80.h"

#define FLAG_CARRY     0
#define FLAG_SUBTRACT  1
#define FLAG_PARITY    2
#define FLAG_OVERFLOW  2
#define FLAG_HALFCARRY 4
#define FLAG_ZERO      6
#define FLAG_SIGN      7

/*
    Initialize a Z80 object.
*/
void z80_init(Z80 *z80, uint64_t clock_speed)
{
    z80->clock_speed = clock_speed;
}

/*
    Power on the Z80, setting registers to their default values.
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
}

/*
    Return whether a particular flag is set in the F register.
*/
static inline bool get_flag(Z80 *z80, uint8_t flag)
{
    return z80->regfile.f & (1 << flag);
}

/*
    Return whether a particular flag is set in the F' register.
*/
static inline bool get_shadow_flag(Z80 *z80, uint8_t flag)
{
    return z80->regfile.f_ & (1 << flag);
}

/*
    Return the CPU's current interrupt mode.
*/
static inline uint8_t get_interrupt_mode(Z80 *z80)
{
    if (!z80->regfile.im_a)
        return 0;
    if (!z80->regfile.im_b)
        return 1;
    return 2;
}

#ifdef DEBUG_MODE
/*
    DEBUG FUNCTION: Print out all register values to stdout.
*/
void z80_dump_registers(Z80 *z80)
{
    Z80RegFile *regfile = &z80->regfile;
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

    DEBUG("- IM:   0b%u%u (mode: %u)", regfile->im_a, regfile->im_b, get_interrupt_mode(z80))
    DEBUG("- IFF1: %u", regfile->iff1)
    DEBUG("- IFF2: %u", regfile->iff2)
}
#endif
