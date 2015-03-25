/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include "rom.h"

/* Structs */

typedef struct {
    ROM *rom;
} GameGear;

/* Functions */

GameGear* gamegear_create();
void gamegear_destroy(GameGear*);
void gamegear_load(GameGear*, ROM*);
void gamegear_power(GameGear*);
