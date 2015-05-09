/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "util_alloc.h"

#define INVALID_SIZE_CODE 0x8

/* Functions */

uint8_t bcd_encode(uint8_t);
uint8_t bcd_decode(uint8_t);
uint64_t get_time_ns();
bool is_valid_symbol_char(char, bool);
const char* region_code_to_string(uint8_t);
uint8_t region_string_to_code(const char*);
size_t size_code_to_bytes(uint8_t);
uint8_t size_bytes_to_code(size_t);
uint16_t compute_checksum(const uint8_t*, size_t, uint8_t);
const char* get_third_party_developer(uint8_t);
