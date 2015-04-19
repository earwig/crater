/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdlib.h>

#include "state.h"
#include "io.h"
#include "../logging.h"

/*
    Initialize default values in an AssemblerState object.
*/
void state_init(AssemblerState *state)
{
    state->header.offset = DEFAULT_HEADER_OFFSET;
    state->header.checksum = true;
    state->header.product_code = 0;
    state->header.version = 0;
    state->header.region = DEFAULT_REGION;
    state->header.rom_size = DEFAULT_DECLSIZE;
    state->optimizer = false;
    state->rom_size = 0;

    state->lines = NULL;
    state->includes = NULL;
    state->instructions = NULL;
    state->symtable = NULL;
}

/*
    Initialize an ASMSymbolTable and place it in *symtable_ptr.
*/
void asm_symtable_init(ASMSymbolTable **symtable_ptr)
{
    ASMSymbolTable *symtable;
    if (!(symtable = malloc(sizeof(ASMSymbolTable))))
        OUT_OF_MEMORY()

    for (size_t bucket = 0; bucket < SYMBOL_TABLE_BUCKETS; bucket++)
        symtable->buckets[bucket] = NULL;

    *symtable_ptr = symtable;
}

/*
    Deallocate an ASMLine list.
*/
void asm_lines_free(ASMLine *line)
{
    while (line) {
        ASMLine *temp = line->next;
        free(line->data);
        free(line);
        line = temp;
    }
}

/*
    Deallocate an ASMInclude list.
*/
void asm_includes_free(ASMInclude *include)
{
    while (include) {
        ASMInclude *temp = include->next;
        line_buffer_free(include->lines);
        free(include);
        include = temp;
    }
}

/*
    Deallocate an ASMInstruction list.
*/
void asm_instructions_free(ASMInstruction *inst)
{
    while (inst) {
        ASMInstruction *temp = inst->next;
        if (inst->symbol)
            free(inst->symbol);
        free(inst);
        inst = temp;
    }
}

/*
    Deallocate an ASMSymbolTable.
*/
void asm_symtable_free(ASMSymbolTable *symtable)
{
    if (!symtable)
        return;

    for (size_t bucket = 0; bucket < SYMBOL_TABLE_BUCKETS; bucket++) {
        ASMSymbol *sym = symtable->buckets[bucket], *temp;
        while (sym) {
            temp = sym->next;
            free(sym->symbol);
            free(sym);
            sym = temp;
        }
    }
    free(symtable);
}
