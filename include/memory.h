#ifndef ASBTL_MEMORY_H
#define ASBTL_MEMORY_H

#include <stddef.h>

#define GROW_CAPACITY(cap) ((cap) < 8 ? 8 : (cap) * 2)

#define GROW_ARRAY(type, ptr, oldCap, newCap) \
  (type *)reallocate(ptr, sizeof(type) * (newCap), sizeof(type) * (oldCap))

#define FREE_ARRAY(type, ptr, oldCap) \
  reallocate(ptr, 0, sizeof(type) * (oldCap))

void *reallocate(void *ptr, size_t newSize, size_t oldSize);

#endif
