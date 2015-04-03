/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/* Error strings */

static const char* rom_err_isdir     = "Is a directory";
static const char* rom_err_notfile   = "Is not a regular file";
static const char* rom_err_badsize   = "Invalid size";
static const char* rom_err_badread   = "Couldn't read the entire file";
static const char* rom_err_badheader = "Invalid header";

/* Structs */

typedef struct {
    char *name;
    uint8_t *data;
    size_t size;
    uint16_t checksum;
    bool valid_checksum;
    uint32_t product_code;
    uint8_t version;
    uint8_t region_code;
} ROM;

/* Functions */

const char* rom_open(ROM**, const char*);
void rom_close(ROM*);
const char* rom_region(const ROM*);
