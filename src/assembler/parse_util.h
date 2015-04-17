/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "state.h"

bool parse_bool(bool*, const ASMLine*, const char*, bool);
bool parse_uint32(uint32_t*, const ASMLine*, const char*);
