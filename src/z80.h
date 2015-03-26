/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stdint.h>

/* Structs */

typedef struct {
    uint8_t  a,  f,  b,  c,  d,  e,  h,  l;
    uint8_t  a_, f_, b_, c_, d_, e_, h_, l_;
    uint16_t ix, iy, sp, pc;
    uint8_t  i,  r;
    bool     im_a, im_b;
    bool     iff1, iff2;
} Z80RegFile;

typedef struct {
    Z80RegFile regfile;
    uint64_t clock_speed;
} Z80;

/* Functions */

void z80_init(Z80*, uint64_t);
void z80_power(Z80*);

#ifdef DEBUG_MODE
void z80_dump_registers(Z80*);
#endif
