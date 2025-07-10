#ifndef ASBTL_MEMORY_H
#define ASBTL_MEMORY_H

#include "value.h"

#include <stddef.h>

// malloc
#define ALLOCATE(type, count) (type *)reallocate(NULL, sizeof(type) * count, 0)

#define GROW_CAPACITY(cap)    ((cap) < 8 ? 8 : (cap) * 2)

#define GROW_ARRAY(type, ptr, oldCap, newCap) \
  (type *)reallocate(ptr, sizeof(type) * (newCap), sizeof(type) * (oldCap))

#define FREE_ARRAY(type, ptr, oldCap) \
  reallocate(ptr, 0, sizeof(type) * (oldCap))

// free
#define FREE(type, ptr) reallocate(ptr, 0, sizeof(type))

void *reallocate(void *ptr, size_t newSize, size_t oldSize);
void markValue(Value value);
void markObj(Obj *obj);
void collectGarbage();
void freeObjs();

#endif
