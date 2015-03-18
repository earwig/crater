/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

/* Structs */

typedef struct {
    char* name;
    char* data;
} rom_type;

/* Functions */

rom_type* rom_open(const char*);
void rom_close(rom_type*);
