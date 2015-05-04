/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "inst_args.h"
#include "state.h"

#define dparse__Bool dparse_bool

/* Functions */

/* General parsers */
bool parse_bool(bool*, const char*, ssize_t);
bool parse_uint32_t(uint32_t*, const char*, ssize_t);
bool parse_string(char**, size_t*, const char*, ssize_t);
bool parse_bytes(uint8_t**, size_t*, const char*, ssize_t);

/* Instruction argument parsers */
bool argparse_register(ASMArgRegister*, const char*, ssize_t);
bool argparse_condition(ASMArgCondition*, const char*, ssize_t);
bool argparse_immediate(ASMArgImmediate*, const char*, ssize_t);
bool argparse_indirect(ASMArgIndirect*, const char*, ssize_t);
bool argparse_indexed(ASMArgIndexed*, const char*, ssize_t);

/* Preprocessor directive parsers */
bool dparse_bool(bool*, const ASMLine*, const char*);
bool dparse_uint32_t(uint32_t*, const ASMLine*, const char*);
bool dparse_uint16_t(uint16_t*, const ASMLine*, const char*);
bool dparse_uint8_t(uint8_t*, const ASMLine*, const char*);
bool dparse_rom_size(uint32_t*, const ASMLine*, const char*);
bool dparse_region_string(uint8_t*, const ASMLine*, const char*);
bool dparse_size_code(uint8_t*, const ASMLine*, const char*);
