/* Copyright (C) 2014-2017 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "mmu.h"
#include "psg.h"
#include "vdp.h"

/* Structs */

typedef struct {
    MMU *mmu;
    VDP *vdp;
    PSG *psg;
    uint8_t ports[6];
    uint8_t buttons;
    bool start;
} IO;

/* Functions */

void io_init(IO*, MMU*, VDP*, PSG*);
void io_power(IO*);
bool io_check_irq(IO*);
void io_set_button(IO*, uint8_t, bool);
void io_set_start(IO*, bool);
uint8_t io_port_read(IO*, uint8_t);
void io_port_write(IO*, uint8_t, uint8_t);
