#ifndef ASBTL_HASHTABLE_H
#define ASBTL_HASHTABLE_H

#include "object.h"
#include "value.h"

typedef struct hash_table_entry {
  ObjString *key;
  Value value;
} HashTableEntry;

#define IS_EMPTY_ENTRY(entry) ((entry)->key == NULL && IS_NIL((entry)->value))

#define MAKE_TOMBSTONE(entry)        \
  do {                               \
    (entry)->key   = NULL;           \
    (entry)->value = BOOL_VAL(true); \
  } while (false)

typedef struct hash_table {
  unsigned int capacity;
  unsigned int count; // Real entries + tombstones
  HashTableEntry *entries;
} HashTable;

void initHashTable(HashTable *ht);
void freeHashTable(HashTable *ht);

// Adds the given key/value pair, returning true if it is a new entry.
bool hashTableSet(HashTable *ht, ObjString *key, Value value);

// Finds the entry with the given key, setting the value out pointer and
// returning true if found.
bool hashTableGet(HashTable *ht, ObjString *key, Value *value);

// Removes the entry with the associated key, marking it as a tombstone entry.
// Returns true if successful - there is an entry and it has been tombstoned.
bool hashTableRemove(HashTable *ht, ObjString *key);

ObjString *tableFindString(HashTable *ht, const char *key, int n,
                           uint32_t hash);

#endif
