/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdlib.h>
#include <string.h>

#include "hash_table.h"
#include "../logging.h"

#define INITIAL_BUCKETS 127

#define GET_FIELD_(obj, offset, type) (*((type*) (((char*) obj) + offset)))

#define NODE_KEY(tab, node) GET_FIELD_(node, tab->key_offset, char*)
#define NEXT_NODE(tab, node) GET_FIELD_(node, tab->next_offset, HashNode*)

/* Internal structs */

struct HashNode {
    char *key;
    HashNode *next;
};

/*
    Hash a string key into a hash table bucket index.

    This uses the djb2 algorithm: http://www.cse.yorku.ca/~oz/hash.html
*/
static inline size_t hash_key(const HashTable *table, const char *key)
{
    size_t hash = 5381;
    while (*key)
        hash = ((hash << 5) + hash) + *(key++);
    return hash % table->buckets;
}

/*
    Create and return a new HashTable.
*/
HashTable* hash_table_new(size_t key_offset, size_t next_offset)
{
    HashTable *table;
    if (!(table = malloc(sizeof(HashTable))))
        OUT_OF_MEMORY()

    if (!(table->nodes = calloc(INITIAL_BUCKETS, sizeof(HashNode*))))
        OUT_OF_MEMORY()

    table->buckets = INITIAL_BUCKETS;
    table->key_offset = key_offset;
    table->next_offset = next_offset;
    return table;
}

/*
    Deallocate a HashTable. This function does nothing if the table is NULL.

    The given callback function is called on each node as it is removed from
    the table. Typically, it will free() the node's members and then free()
    the node itself.
*/
void hash_table_free(HashTable *table, HashFreeCallback callback)
{
    if (!table)
        return;

    for (size_t bucket = 0; bucket < table->buckets; bucket++) {
        HashNode *node = table->nodes[bucket];
        while (node) {
            HashNode *temp = NEXT_NODE(table, node);
            callback(node);
            node = temp;
        }
    }
    free(table);
}

/*
    Search for a key in the hash table.

    Return the corresponding node on success and NULL on failure.
*/
const HashNode* hash_table_find(const HashTable *table, const char *key)
{
    HashNode *node = table->nodes[hash_key(table, key)];
    while (node) {
        if (!strcmp(key, NODE_KEY(table, node)))
            return node;
        node = NEXT_NODE(table, node);
    }
    return NULL;
}

/*
    Insert a node into the table.

    This doesn't check for duplicate keys, so you must do that beforehand.
*/
void hash_table_insert(HashTable *table, HashNode *node)
{
    size_t index = hash_key(table, NODE_KEY(table, node));
    NEXT_NODE(table, node) = table->nodes[index];
    table->nodes[index] = node;
}
