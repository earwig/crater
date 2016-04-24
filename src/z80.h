/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "mmu.h"

#define Z80_EXC_NOT_POWERED          0
#define Z80_EXC_UNIMPLEMENTED_OPCODE 1
#define Z80_EXC_UNIMPLEMENTED_PORT   2

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
    bool fresh;
    uint16_t last_addr;
    uint64_t counter;
} Z80TraceInfo;

typedef struct {
    Z80RegFile regfile;
    MMU *mmu;
    bool except;
    uint8_t exc_code, exc_data;
    double pending_cycles;
    Z80TraceInfo trace;
} Z80;

/* Functions */

void z80_init(Z80*, MMU*);
void z80_power(Z80*);
bool z80_do_cycles(Z80*, double);
void z80_dump_registers(const Z80*);
