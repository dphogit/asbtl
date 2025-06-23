#include "chunk.h"

#include "memory.h"
#include "value.h"

#include <stddef.h>
#include <stdint.h>

const char *opCodeStr(OpCode opCode) {
  switch (opCode) {
    case OP_ADD:        return "OP_ADD";
    case OP_SUBTRACT:   return "OP_SUBTRACT";
    case OP_MULTIPLY:   return "OP_MULTIPLY";
    case OP_DIVIDE:     return "OP_DIVIDE";
    case OP_FALSE:      return "OP_FALSE";
    case OP_TRUE:       return "OP_TRUE";
    case OP_NOT:        return "OP_NOT";
    case OP_NEGATE:     return "OP_NEGATE";
    case OP_NIL:        return "OP_NIL";
    case OP_EQ:         return "OP_EQ";
    case OP_NOT_EQ:     return "OP_NOT_EQ";
    case OP_LESS:       return "OP_LESS";
    case OP_LESS_EQ:    return "OP_LESS_EQ";
    case OP_GREATER:    return "OP_GREATER";
    case OP_GREATER_EQ: return "OP_GREATER_EQ";
    case OP_CONSTANT:   return "OP_CONSTANT";
    case OP_RETURN:     return "OP_RETURN";
  }
  return "unknown opcode";
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
