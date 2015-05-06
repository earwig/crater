/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stddef.h>

#define hash_table_NEW(node, key, next, callback)                             \
    hash_table_new(offsetof(node, key), offsetof(node, next),                 \
                   (HashFreeCallback) callback)

/* Structs */

typedef struct HashNode HashNode;

typedef void (*HashFreeCallback)(HashNode*);

typedef struct {
    HashNode **nodes;
    size_t buckets;
    size_t key_offset;
    size_t next_offset;
    HashFreeCallback free;
} HashTable;

/* Functions */

HashTable* hash_table_new(size_t, size_t, HashFreeCallback);
void hash_table_free(HashTable*);
const HashNode* hash_table_find(const HashTable*, const char*);
void hash_table_insert(HashTable*, HashNode*);
bool hash_table_remove(HashTable*, const char*);
