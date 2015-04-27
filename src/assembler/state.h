/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../assembler.h"

#define DEFAULT_HEADER_OFFSET 0x7FF0
#define DEFAULT_REGION 6  // GG Export
#define DEFAULT_DECLSIZE 0xC  // 32 KB

#define SYMBOL_TABLE_BUCKETS 128

/* Structs */

struct ASMLine {
    char *data;
    size_t length;
    const Line *original;
    const char *filename;
    bool is_label;
    struct ASMLine *next;
};
typedef struct ASMLine ASMLine;

struct ASMInclude {
    LineBuffer *lines;
    struct ASMInclude *next;
};
typedef struct ASMInclude ASMInclude;

typedef struct {
    size_t offset;
    size_t length;
} ASMLocation;

struct ASMInstruction {
    ASMLocation loc;
    uint8_t b1, b2, b3, b4;
    char *symbol;
    const ASMLine *line;
    struct ASMInstruction *next;
};
typedef struct ASMInstruction ASMInstruction;

struct ASMData {
    ASMLocation loc;
    uint8_t *data;
    struct ASMData *next;
};
typedef struct ASMData ASMData;

struct ASMSymbol {
    uint16_t offset;
    char *symbol;
    const ASMLine *line;
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
    bool cross_blocks;
    size_t rom_size;
    ASMLine *lines;
    ASMInclude *includes;
    ASMInstruction *instructions;
    ASMData *data;
    ASMSymbolTable *symtable;
} AssemblerState;

/* Functions */

void state_init(AssemblerState*);
void state_free(AssemblerState*);
void asm_symtable_init(ASMSymbolTable**);
void asm_lines_free(ASMLine*);
void asm_includes_free(ASMInclude*);
void asm_instructions_free(ASMInstruction*);
void asm_data_free(ASMData*);
void asm_symtable_free(ASMSymbolTable*);

const ASMSymbol* asm_symtable_find(const ASMSymbolTable*, const char*);
void asm_symtable_insert(ASMSymbolTable*, ASMSymbol*);

#ifdef DEBUG_MODE
void asm_lines_print(const ASMLine*);
#endif
