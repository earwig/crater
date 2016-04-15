/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hash_table.h"
#include "inst_args.h"
#include "../assembler.h"

#define DEFAULT_HEADER_OFFSET 0x7FF0
#define DEFAULT_REGION 6  // GG Export
#define DEFAULT_DECLSIZE 0xC  // 32 KB

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
    uint8_t *bytes;
    char *symbol;
    const ASMLine *line;
    struct ASMInstruction *next;
};
typedef struct ASMInstruction ASMInstruction;

struct ASMData {
    ASMLocation loc;
    uint8_t *bytes;
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

struct ASMDefine {
    ASMArgImmediate value;
    char *name;
    const ASMLine *line;
    struct ASMDefine *next;
};
typedef struct ASMDefine ASMDefine;

typedef HashTable ASMSymbolTable;
typedef HashTable ASMDefineTable;

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
ASMDefineTable* asm_deftable_new();
void asm_lines_free(ASMLine*);
void asm_includes_free(ASMInclude*);
void asm_instructions_free(ASMInstruction*);
void asm_data_free(ASMData*);
void asm_symtable_free(ASMSymbolTable*);
void asm_deftable_free(ASMDefineTable*);

const ASMSymbol* asm_symtable_find(const ASMSymbolTable*, const char*);
void asm_symtable_insert(ASMSymbolTable*, ASMSymbol*);
const ASMDefine* asm_deftable_find(const ASMDefineTable*, const char*, size_t);
void asm_deftable_insert(ASMDefineTable*, ASMDefine*);
bool asm_deftable_remove(ASMDefineTable*, const char*, size_t);

void asm_lines_print(const ASMLine*);
