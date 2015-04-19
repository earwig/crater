/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "state.h"

#define parse_bool parse__Bool

/* Functions */

bool parse__Bool(bool*, const ASMLine*, const char*);
bool parse_uint32_t(uint32_t*, const ASMLine*, const char*);
bool parse_uint16_t(uint16_t*, const ASMLine*, const char*);
bool parse_uint8_t(uint8_t*, const ASMLine*, const char*);

bool parse_rom_size(uint32_t*, const ASMLine*, const char*);
bool parse_region_string(uint8_t*, const ASMLine*, const char*);
bool parse_size_code(uint8_t*, const ASMLine*, const char*);
