/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdint.h>
#include <stdlib.h>

#define ROM_SIZE_MIN (32 << 10)  // 32 KB
#define ROM_SIZE_MAX ( 1 << 20)  //  1 MB

/* Header info */

#define HEADER_SIZE 16
#define HEADER_MAGIC_LEN 8

static const char rom_header_magic[HEADER_MAGIC_LEN + 1] = "TMR SEGA";

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
    uint16_t reported_checksum;
    uint16_t expected_checksum;
    uint32_t product_code;
    uint8_t version;
    uint8_t region_code;
} ROM;

/* Functions */

const char* rom_open(ROM**, const char*);
void rom_close(ROM*);
const char* rom_product(const ROM*);
const char* rom_region(const ROM*);
