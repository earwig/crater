/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "mmu.h"
#include "rom.h"
#include "z80.h"

#define GG_EXC_BUFF_SIZE 128

/* Structs */

typedef struct {
    MMU mmu;
    Z80 cpu;
    bool powered;
    uint64_t last_tick;
    char exc_buffer[GG_EXC_BUFF_SIZE];
} GameGear;

/* Functions */

GameGear* gamegear_create();
void gamegear_destroy(GameGear*);
void gamegear_load(GameGear*, ROM*);
void gamegear_power(GameGear*, bool);
bool gamegear_simulate(GameGear*);
const char* gamegear_get_exception(GameGear*);
