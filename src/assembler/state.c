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
    state->data = NULL;
    state->symtable = NULL;
}

/*
    Deallocate the contents of an AssemblerState object.
*/
void state_free(AssemblerState *state)
{
    asm_lines_free(state->lines);
    asm_includes_free(state->includes);
    asm_instructions_free(state->instructions);
    asm_data_free(state->data);
    asm_symtable_free(state->symtable);
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
    Deallocate an ASMData list.
*/
void asm_data_free(ASMData *data)
{
    while (data) {
        ASMData *temp = data->next;
        free(data->data);
        free(data);
        data = temp;
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

/*
    Hash a string key into a symbol table bucket index.

    This uses the djb2 algorithm: http://www.cse.yorku.ca/~oz/hash.html
*/
static inline size_t hash_key(const char *key)
{
    size_t hash = 5381;
    while (*key)
        hash = ((hash << 5) + hash) + *(key++);
    return hash % SYMBOL_TABLE_BUCKETS;
}

/*
    Search for a key in the symbol table.

    Return the corresponding symbol on success and NULL on failure.
*/
const ASMSymbol* asm_symtable_find(const ASMSymbolTable *tab, const char *key)
{
    ASMSymbol *symbol = tab->buckets[hash_key(key)];
    while (symbol) {
        if (!strcmp(key, symbol->symbol))
            return symbol;
        symbol = symbol->next;
    }
    return NULL;
}

/*
    Insert a symbol into the table.

    TODO: return boolean on success instead of void.
*/
void asm_symtable_insert(ASMSymbolTable *tab, ASMSymbol *symbol)
{
    size_t index = hash_key(symbol->symbol);
    symbol->next = tab->buckets[index];
    tab->buckets[index] = symbol;
}

#ifdef DEBUG_MODE
/*
    DEBUG FUNCTION: Print out an ASMLine list to stdout.
*/
void asm_lines_print(const ASMLine *line)
{
    DEBUG("Dumping ASMLines:")
    while (line) {
        DEBUG("- %-40.*s [%s:%02zu]", (int) line->length, line->data,
              line->filename, line->original->lineno)
        line = line->next;
    }
}
#endif
