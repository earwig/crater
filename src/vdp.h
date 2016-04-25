/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stdint.h>

/* Structs */

typedef struct {
    uint8_t  control_code;
    uint16_t control_addr;
    bool     control_flag;
} VDP;

/* Functions */

void vdp_init(VDP*);
void vdp_free(VDP*);
void vdp_power(VDP*);

uint8_t vdp_read_control(VDP*);
uint8_t vdp_read_data(VDP*);
void vdp_write_control(VDP*, uint8_t);
void vdp_write_data(VDP*, uint8_t);
