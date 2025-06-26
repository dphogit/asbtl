#include "hashtable.h"
#include "memory.h"
#include "value.h"

#include <stddef.h>
#include <string.h>

#define MAX_LOAD_FACTOR 0.75

void initHashTable(HashTable *ht) {
  ht->capacity = 0;
  ht->count    = 0;
  ht->entries  = NULL;
}

void freeHashTable(HashTable *ht) {
  FREE_ARRAY(HashTableEntry, ht->entries, ht->capacity);
  initHashTable(ht);
}

static HashTableEntry *findEntry(HashTableEntry *entries, ObjString *key,
                                 unsigned int capacity) {
  uint32_t index            = key->hash % capacity;
  HashTableEntry *tombstone = NULL;

  while (true) {
    HashTableEntry *entry = &entries[index];

    // Empty slot
    if (entry->key == NULL) {
      if (IS_NIL(entry->value)) {
        // Real empty slot: return tombstone if we found one, else this slot.
        // Allows the reuse of tombstone entries.
        return tombstone != NULL ? tombstone : entry;
      }

      // Remember the first tombstone
      if (tombstone == NULL) {
        tombstone = entry;
      }
    }

    // Key match - TODO: Replace with reference equality - string interning
    else if (entry->key->len == key->len &&
             strncmp(entry->key->chars, key->chars, key->len) == 0) {
      return entry;
    }

    index = (index + 1) % capacity; // open addressing probing strategy
  }
}

static void adjustCapacity(HashTable *ht, unsigned int newCap) {
  // Allocate new buckets
  HashTableEntry *entries = ALLOCATE(HashTableEntry, newCap);
  for (unsigned int i = 0; i < newCap; i++) {
    entries[i].key   = NULL;
    entries[i].value = NIL_VAL;
  }

  // Rebuild table from scratch by recalculating/reinserting existing entries
  ht->count = 0;
  for (unsigned int i = 0; i < ht->capacity; i++) {
    HashTableEntry *entry = &ht->entries[i];

    if (entry->key == NULL)
      continue;

    HashTableEntry *dest = findEntry(entries, entry->key, newCap);
    dest->key            = entry->key;
    dest->value          = entry->value;
    ht->count++;
  }

  // Update table metadata
  FREE_ARRAY(HashTableEntry, ht->entries, ht->capacity);
  ht->entries  = entries;
  ht->capacity = newCap;
}

bool hashTableSet(HashTable *ht, ObjString *key, Value value) {
  if (ht->count >= ht->capacity * MAX_LOAD_FACTOR) {
    adjustCapacity(ht, GROW_CAPACITY(ht->capacity));
  }

  HashTableEntry *entry = findEntry(ht->entries, key, ht->capacity);
  bool isNewEntry       = entry->key == NULL;

  if (IS_EMPTY_ENTRY(entry)) {
    ht->count++;
  }

  entry->key   = key;
  entry->value = value;

  return isNewEntry;
}

bool hashTableGet(HashTable *ht, ObjString *key, Value *out) {
  if (ht->count == 0)
    return false;

  HashTableEntry *entry = findEntry(ht->entries, key, ht->capacity);
  if (entry->key == NULL)
    return false;

  *out = entry->value;
  return true;
}

bool hashTableRemove(HashTable *ht, ObjString *key) {
  if (ht->count == 0)
    return false;

  HashTableEntry *entry = findEntry(ht->entries, key, ht->capacity);
  if (entry == NULL)
    return false;

  MAKE_TOMBSTONE(entry);
  return true;
}
