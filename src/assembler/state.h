/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../assembler.h"

#define SYMBOL_TABLE_BUCKETS 128

/* Structs */

struct ASMLine {
    char *data;
    size_t length;
    const Line *original;
    const char *filename;
    struct ASMLine *next;
};
typedef struct ASMLine ASMLine;

struct ASMInclude {
    LineBuffer *lines;
    struct ASMInclude *next;
};
typedef struct ASMInclude ASMInclude;

struct ASMInstruction {
    size_t offset;
    uint8_t length;
    uint8_t b1, b2, b3, b4;
    uint8_t virtual_byte;
    char *symbol;
    struct ASMInstruction *next;
};
typedef struct ASMInstruction ASMInstruction;

struct ASMSymbol {
    size_t offset;
    char *symbol;
    struct ASMSymbol *next;
};
typedef struct ASMSymbol ASMSymbol;

typedef struct {
    ASMSymbol *buckets[SYMBOL_TABLE_BUCKETS];
} ASMSymbolTable;

typedef struct {
    size_t offset;
    bool checksum;
    uint32_t product_code;
    uint8_t version;
    uint8_t region;
    uint8_t rom_size;
} ASMHeaderInfo;

typedef struct {
    ASMHeaderInfo header;
    bool optimizer;
    size_t rom_size;
    ASMLine *lines;
    ASMInclude *includes;
    ASMInstruction *instructions;
    ASMSymbolTable *symtable;
} AssemblerState;

/* Functions */

void state_init(AssemblerState*);
void asm_lines_free(ASMLine*);
void asm_includes_free(ASMInclude*);
void asm_instructions_free(ASMInstruction*);
void asm_symtable_free(ASMSymbolTable*);
