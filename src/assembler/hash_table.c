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
static inline size_t hash_key(
    const HashTable *table, const char *key, ssize_t size)
{
    size_t hash = 5381;
    while ((size < 0 || size-- > 0) && *key)
        hash = ((hash << 5) + hash) + *(key++);
    return hash % table->buckets;
}

/*
    Return true if the two key strings are equal, or false if they aren't.
*/
static inline bool keyeq(const char *s1, const char *s2, ssize_t size)
{
    return !(size >= 0 ? strncmp(s1, s2, size) : strcmp(s1, s2));
}

/*
    Create and return a new HashTable.

    These HashTables are designed to be generic, a sort of poor-man's C++
    template. They can store any kind of node data as long as they are structs
    with a char* field (for the node key), and a pointer to its own type (to
    implement separate chaining).

    key_offset is the (byte) offset of the key field, and next_offset is the
    offset of the self-pointer. The callback function is called on a node when
    it is removed from the table. Typically, it should free() the node's
    members and then free() the node itself.

    The hash_table_NEW macro can be used to call this function more easily.
*/
HashTable* hash_table_new(
    size_t key_offset, size_t next_offset, HashFreeCallback callback)
{
    HashTable *table;
    if (!(table = malloc(sizeof(HashTable))))
        OUT_OF_MEMORY()

    if (!(table->nodes = calloc(INITIAL_BUCKETS, sizeof(HashNode*))))
        OUT_OF_MEMORY()

    table->buckets = INITIAL_BUCKETS;
    table->key_offset = key_offset;
    table->next_offset = next_offset;
    table->free = callback;
    return table;
}

/*
    Deallocate a HashTable. This function does nothing if the table is NULL.
*/
void hash_table_free(HashTable *table)
{
    if (!table)
        return;

    for (size_t bucket = 0; bucket < table->buckets; bucket++) {
        HashNode *node = table->nodes[bucket];
        while (node) {
            HashNode *temp = NEXT_NODE(table, node);
            table->free(node);
            node = temp;
        }
    }
    free(table);
}

/*
    Search for a key in the hash table.

    Return the corresponding node on success and NULL on failure.

    If the key is null-terminated, pass size as -1.
*/
const HashNode* hash_table_find(
    const HashTable *table, const char *key, ssize_t size)
{
    HashNode *node = table->nodes[hash_key(table, key, size)];
    while (node) {
        if (keyeq(key, NODE_KEY(table, node), size))
            return node;
        node = NEXT_NODE(table, node);
    }
    return NULL;
}

/*
    Insert a node into the table.

    This doesn't prevent inserting duplicate keys. If a duplicate key is
    inserted, the table acts like a stack; the new one will shadow the old one,
    and hash_table_remove() will remove the most-recently inserted key to
    reveal the second-most recent.
*/
void hash_table_insert(HashTable *table, HashNode *node)
{
    size_t index = hash_key(table, NODE_KEY(table, node), -1);
    NEXT_NODE(table, node) = table->nodes[index];
    table->nodes[index] = node;
}

/*
    (Try to) remove a node with the given key from the table.

    Return true if the node was removed, or false if it was not found.

    If the key is null-terminated, pass size as -1.
*/
bool hash_table_remove(HashTable *table, const char *key, ssize_t size)
{
    size_t index = hash_key(table, key, size);
    HashNode *node = table->nodes[index], *prev = NULL, *next;

    while (node) {
        next = NEXT_NODE(table, node);
        if (keyeq(key, NODE_KEY(table, node), size)) {
            if (prev)
                NEXT_NODE(table, prev) = next;
            else
                table->nodes[index] = next;
            table->free(node);
            return true;
        }
        prev = node;
        node = next;
    }
    return false;
}
