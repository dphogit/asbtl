#include "chunk.h"

#include "memory.h"

#include <stddef.h>
#include <stdint.h>

void initChunk(Chunk *chunk) {
  chunk->capacity  = 0;
  chunk->count     = 0;
  chunk->code      = NULL;
  chunk->constants = NULL;
}

void appendChunk(Chunk *chunk, uint8_t byte) {
  if (chunk->count >= chunk->capacity) {
    int oldCap      = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(oldCap);
    chunk->code     = GROW_ARRAY(uint8_t, chunk->code, oldCap, chunk->capacity);
  }

  chunk->code[chunk->count] = byte;
  chunk->count++;
}

void freeChunk(Chunk *chunk) {
  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
  initChunk(chunk);
}
