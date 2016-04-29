/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "io.h"
#include "mmu.h"

#define Z80_EXC_NOT_POWERED          0
#define Z80_EXC_UNIMPLEMENTED_OPCODE 1
#define Z80_EXC_IO_ERROR             2

/* Structs */

#ifdef __BIG_ENDIAN__
#define REG1(hi, lo) hi
#define REG2(hi, lo) lo
#else
#define REG1(hi, lo) lo
#define REG2(hi, lo) hi
#endif

#define REG_PAIR(hi, lo, pair)    \
    union {                       \
        struct {                  \
            uint8_t REG1(hi, lo); \
            uint8_t REG2(hi, lo); \
        };                        \
        uint16_t pair;            \
    };

typedef struct {
    REG_PAIR(a, f, af)
    REG_PAIR(b, c, bc)
    REG_PAIR(d, e, de)
    REG_PAIR(h, l, hl)

    REG_PAIR(a_, f_, af_)
    REG_PAIR(b_, c_, bc_)
    REG_PAIR(d_, e_, de_)
    REG_PAIR(h_, l_, hl_)

    REG_PAIR(ixh, ixl, ix)
    REG_PAIR(iyh, iyl, iy)

    uint16_t sp, pc;
    uint8_t  i,  r;
    bool     im_a, im_b;
    bool     iff1, iff2;
} Z80RegFile;

typedef struct {
    bool fresh;
    uint16_t last_addr;
    uint64_t counter;
} Z80TraceInfo;

typedef struct {
    Z80RegFile regs;
    MMU *mmu;
    IO *io;
    bool except;
    uint8_t exc_code, exc_data;
    uint16_t *last_index;
    double pending_cycles;
    Z80TraceInfo trace;
} Z80;

#undef REG_PAIR
#undef REG1
#undef REG2

/* Functions */

void z80_init(Z80*, MMU*, IO*);
void z80_power(Z80*);
bool z80_do_cycles(Z80*, double);
void z80_dump_registers(const Z80*);
