#include "chunk.h"

#include "memory.h"
#include "value.h"

#include <stddef.h>
#include <stdint.h>

const char *opCodeStr(OpCode opCode) {
  switch (opCode) {
    case OP_ADD:      return "OP_ADD";
    case OP_SUBTRACT: return "OP_SUBTRACT";
    case OP_CONSTANT: return "OP_CONSTANT";
    case OP_RETURN:   return "OP_RETURN";
    default:          return "unknown opcode";
  }
}

void initChunk(Chunk *chunk) {
  chunk->capacity = 0;
  chunk->count    = 0;
  chunk->code     = NULL;
  chunk->lines    = NULL;
  initValueList(&chunk->constants);
}

void appendChunk(Chunk *chunk, uint8_t byte, int line) {
  if (chunk->count >= chunk->capacity) {
    int oldCap      = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(oldCap);
    chunk->code     = GROW_ARRAY(uint8_t, chunk->code, oldCap, chunk->capacity);
    chunk->lines    = GROW_ARRAY(int, chunk->lines, oldCap, chunk->capacity);
  }

  chunk->code[chunk->count]  = byte;
  chunk->lines[chunk->count] = line;
  chunk->count++;
}

void freeChunk(Chunk *chunk) {
  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
  FREE_ARRAY(int, chunk->lines, chunk->capacity);
  freeValueList(&chunk->constants);
  initChunk(chunk);
}

unsigned int appendConstant(Chunk *chunk, Value constant) {
  appendValueList(&chunk->constants, constant);
  return chunk->constants.count - 1;
}
