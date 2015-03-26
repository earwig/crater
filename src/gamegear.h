/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>

#include "mmu.h"
#include "rom.h"
#include "z80.h"

/* Clock speed in Hz was taken from the official Sega GG documentation */
#define CPU_CLOCK_SPEED 3579545

/* Structs */

typedef struct {
    MMU mmu;
    Z80 cpu;
    bool powered;
} GameGear;

/* Functions */

GameGear* gamegear_create();
void gamegear_destroy(GameGear*);
void gamegear_load(GameGear*, ROM*);
void gamegear_power(GameGear*, bool);
void gamegear_simulate(GameGear*);
