/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stddef.h>

#define hash_table_NEW(node, key, next)                                       \
    hash_table_new(offsetof(node, key), offsetof(node, next))

/* Structs */

typedef struct HashNode HashNode;

typedef struct {
    HashNode **nodes;
    size_t buckets;
    size_t key_offset;
    size_t next_offset;
} HashTable;

typedef void (*HashFreeCallback)(HashNode*);

/* Functions */

HashTable* hash_table_new(size_t, size_t);
void hash_table_free(HashTable*, HashFreeCallback);
const HashNode* hash_table_find(const HashTable*, const char*);
void hash_table_insert(HashTable*, HashNode*);
