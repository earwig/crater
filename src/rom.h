/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

/* Structs */

typedef struct {
    char* name;
    char* data;
} ROM;

/* Functions */

ROM* rom_open(const char*);
void rom_close(ROM*);
