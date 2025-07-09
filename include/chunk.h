#ifndef ASBTL_CHUNK_H
#define ASBTL_CHUNK_H

#include "value.h"

#include <stdint.h>

typedef enum op_code {
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_FALSE,
  OP_TRUE,
  OP_NOT,
  OP_NEGATE,
  OP_NIL,
  OP_EQ,
  OP_NOT_EQ,
  OP_LESS,
  OP_LESS_EQ,
  OP_GREATER_EQ,
  OP_GREATER,
  OP_CONSTANT,
  OP_POP,
  OP_JUMP_IF_FALSE,
  OP_JUMP_IF_TRUE,
  OP_JUMP,
  OP_LOOP,
  OP_DEF_GLOBAL,
  OP_GET_GLOBAL,
  OP_SET_GLOBAL,
  OP_GET_LOCAL,
  OP_SET_LOCAL,
  OP_GET_UPVALUE,
  OP_SET_UPVALUE,
  OP_CLOSE_UPVALUE,
  OP_CALL,
  OP_CLOSURE,
  OP_PRINT,
  OP_RETURN,
} OpCode;

const char *opCodeStr(OpCode opCode);

// A chunk is the abstraction that represents a bytecode array and its
// associated information such as the constants it refers to.
typedef struct chunk {
  unsigned int capacity;
  unsigned int count;
  uint8_t *code;       // Bytecode array
  int *lines;          // Corresponding line numbers of the bytecode
  ValueList constants; // Constants pool (values list)
} Chunk;

void initChunk(Chunk *chunk);
void appendChunk(Chunk *chunk, uint8_t byte, int line);
void freeChunk(Chunk *chunk);

// Appends to the constant pool, returning the array index it was written to.
unsigned int appendConstant(Chunk *chunk, Value constant);

#endif
