/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "rom.h"

/* Structs */

typedef struct {
    size_t size;
    char *bytestr;
    char *line;
} DisasInstr;

/* Functions */

void disas_instr_free(DisasInstr*);

DisasInstr* disassemble_instruction(const uint8_t*);
char** disassemble(const ROM*);
bool disassemble_file(const char*, const char*);
