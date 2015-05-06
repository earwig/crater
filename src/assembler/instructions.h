/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdint.h>

#include "errors.h"
#include "parse_util.h"

#define MIN_MNEMONIC_SIZE 2
#define MAX_MNEMONIC_SIZE 4

/* Typedefs */

typedef ASMErrorDesc (*ASMInstParser)(
    uint8_t**, size_t*, char**, ASMArgParseInfo);

/* Functions */

ASMInstParser get_inst_parser(char[MAX_MNEMONIC_SIZE]);
