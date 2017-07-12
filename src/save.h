/* Copyright (C) 2014-2017 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "rom.h"

/* Structs */

typedef struct {
    char *path;
    const ROM *rom;
    void *map;
    size_t mapsize;
    size_t cart_ram_offset;
    bool has_cart_ram;
} Save;

/* Functions */

bool save_init(Save*, const char*, const ROM*);
void save_free(Save*);

bool save_has_cart_ram(const Save*);
uint8_t* save_get_cart_ram(Save*);
bool save_init_cart_ram(Save*);
