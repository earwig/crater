/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

/* Structs */

typedef struct {
    char* name;
    char* data;
} rom_type;

/* Functions */

rom_type* open_rom(const char*);
void close_rom(rom_type*);
