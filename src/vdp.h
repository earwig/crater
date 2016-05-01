/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#define VDP_LINES_PER_FRAME 262
#define VDP_VRAM_SIZE (16 * 1024)
#define VDP_CRAM_SIZE (64)
#define VDP_REGS 11

/* Structs */

typedef struct {
    uint32_t *pixels;

    uint8_t  *vram;
    uint8_t  *cram;
    uint8_t  regs[VDP_REGS];

    uint8_t  h_counter;
    uint8_t  v_counter;
    bool     v_count_jump;

    uint8_t  control_code;
    uint16_t control_addr;
    bool     control_flag;
    bool     stat_int, stat_ovf, stat_col;
    uint8_t  read_buf;
    uint8_t  cram_latch;
} VDP;

/* Functions */

void vdp_init(VDP*);
void vdp_free(VDP*);
void vdp_power(VDP*);
void vdp_simulate_line(VDP*);

uint8_t vdp_read_control(VDP*);
uint8_t vdp_read_data(VDP*);
void vdp_write_control(VDP*, uint8_t);
void vdp_write_data(VDP*, uint8_t);
bool vdp_assert_irq(VDP*);

void vdp_dump_registers(const VDP*);
