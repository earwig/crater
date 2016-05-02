/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "psg.h"
#include "vdp.h"

/* Structs */

typedef struct {
    VDP *vdp;
    PSG *psg;
    uint8_t ports[6];
} IO;

/* Functions */

void io_init(IO*, VDP*, PSG*);
void io_power(IO*);
bool io_check_irq(IO*);
uint8_t io_port_read(IO*, uint8_t);
void io_port_write(IO*, uint8_t, uint8_t);
