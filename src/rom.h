/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

/* Error strings */

static const char* rom_err_isdir   = "Is a directory";
static const char* rom_err_notfile = "Is not a regular file";
static const char* rom_err_badsize = "Invalid size";
static const char* rom_err_badread = "Couldn't read the entire file";

/* Structs */

typedef struct {
    char* name;
    char* data;
    unsigned size;
} ROM;

/* Functions */

const char* rom_open(ROM**, const char*);
void rom_close(ROM*);
