/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "state.h"

#define parse__Bool parse_bool

/* Functions */

bool parse_bool(bool*, const ASMLine*, const char*);
bool parse_uint32_t(uint32_t*, const ASMLine*, const char*);
bool parse_uint16_t(uint16_t*, const ASMLine*, const char*);
bool parse_uint8_t(uint8_t*, const ASMLine*, const char*);

bool parse_region_string(uint8_t*, const ASMLine*);
bool parse_size_code(uint8_t*, const ASMLine*);
