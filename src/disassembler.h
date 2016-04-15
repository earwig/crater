/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Structs */

typedef struct {
    size_t size;
    char *bytestr;
    char *line;
} DisasInstr;

// typedef struct { ... } Disassembly;

/* Functions */

void disas_instr_free(DisasInstr*);

DisasInstr* disassemble_instruction(const uint8_t*);
// Disassembly* disassemble(const uint8_t*);  // TODO
bool disassemble_file(const char*, const char*);
