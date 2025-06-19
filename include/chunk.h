#ifndef ASBTL_CHUNK_H
#define ASBTL_CHUNK_H

#include <stdint.h>

typedef enum op_code {
  OP_PLUS,
  OP_MINUS,
  OP_CONSTANT,
  OP_RETURN,
} OpCode;

// A chunk is the abstraction that represents a bytecode array and its
// associated information such as the constants it refers to.
typedef struct chunk {
  int capacity;
  int count;
  uint8_t *code;  // Bytecode array
  int *constants; // Constants pool
} Chunk;

void initChunk(Chunk *chunk);
void appendChunk(Chunk *chunk, uint8_t byte);
void freeChunk(Chunk *chunk);

#endif
