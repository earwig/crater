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
    state->cross_blocks = false;
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
    Callback function for freeing an ASMSymbol.
*/
static void free_asm_symbol(ASMSymbol *node)
{
    free(node->symbol);
    free(node);
}

/*
    Initialize an ASMSymbolTable and place it in *symtable_ptr.
*/
void asm_symtable_init(ASMSymbolTable **symtable_ptr)
{
    *symtable_ptr = hash_table_NEW(ASMSymbol, symbol, next, free_asm_symbol);
}

/*
    Callback function for freeing an ASMDefine.
*/
static void free_asm_define(ASMDefine *node)
{
    free(node->name);
    free(node);
}

/*
    Create and return a new ASMDefineTable.
*/
ASMDefineTable* asm_deftable_new()
{
    return hash_table_NEW(ASMDefine, name, next, free_asm_define);
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
        free(inst->bytes);
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
        free(data->bytes);
        free(data);
        data = temp;
    }
}

/*
    Deallocate an ASMSymbolTable.
*/
void asm_symtable_free(ASMSymbolTable *symtable)
{
    hash_table_free(symtable);
}


/*
    Deallocate an ASMDefineTable.
*/
void asm_deftable_free(ASMDefineTable *deftable)
{
    hash_table_free(deftable);
}

/*
    Search for a key in the symbol table.

    Return the corresponding symbol on success and NULL on failure.
*/
const ASMSymbol* asm_symtable_find(const ASMSymbolTable *tab, const char *key)
{
    return (ASMSymbol*) hash_table_find(tab, key);
}

/*
    Insert a symbol into the table.

    This doesn't check for duplicate keys, so you must do that beforehand.
*/
void asm_symtable_insert(ASMSymbolTable *tab, ASMSymbol *symbol)
{
    hash_table_insert(tab, (HashNode*) symbol);
}

/*
    Search for a key in the define table.

    Return the corresponding ASMDefine on success and NULL on failure.
*/
const ASMDefine* asm_deftable_find(const ASMDefineTable *tab, const char *key)
{
    return (ASMDefine*) hash_table_find(tab, key);
}

/*
    Insert an ASMDefine into the define table.

    This doesn't check for duplicate keys, so you must do that beforehand.
*/
void asm_deftable_insert(ASMDefineTable *tab, ASMDefine *define)
{
    hash_table_insert(tab, (HashNode*) define);
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
