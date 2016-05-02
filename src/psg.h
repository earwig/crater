/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdint.h>

/* Structs */

typedef struct {
    uint16_t tone1, tone2, tone3;
    uint8_t vol1, vol2, vol3;
    uint8_t noise;
} PSG;

/* Functions */

void psg_init(PSG*);
void psg_free(PSG*);
void psg_power(PSG*);
void psg_write(PSG*, uint8_t);
void psg_stereo(PSG*, uint8_t);
