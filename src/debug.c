#include "debug.h"
#include "chunk.h"

#include <stdio.h>

static unsigned int constant(Chunk *chunk, unsigned int offset) {
  const char *name      = opCodeStr(chunk->code[offset]);
  uint8_t constantIndex = chunk->code[offset + 1];

  printf("%-16s %4d '", name, constantIndex);
  printValue(chunk->constants.values[constantIndex]);
  printf("'\n");

  return offset + 2;
}

static unsigned int byte(Chunk *chunk, unsigned int offset) {
  const char *name = opCodeStr(chunk->code[offset]);
  uint8_t slot     = chunk->code[offset + 1];

  // Unlike global variables, we don't have the variable name but we can at
  // least show it's slot number in the VM if that counts for anything.
  printf("%-16s %4d\n", name, slot);
  return offset + 2;
}

static unsigned int jump(Chunk *chunk, int sign, int offset) {
  const char *name = opCodeStr(chunk->code[offset]);
  uint16_t toJump  = (chunk->code[offset + 1] << 8) | chunk->code[offset + 2];

  int dest = offset + 3 + sign * toJump;
  printf("%-16s %4d -> %d\n", name, offset, dest);

  return offset + 3;
}

static unsigned int single(Chunk *chunk, unsigned int offset) {
  printf("%s\n", opCodeStr(chunk->code[offset]));
  return offset + 1;
}

// Returns the offset of the next instruction in the chunk to disassemble
static unsigned int disassembleInstruction(Chunk *chunk, unsigned int offset) {
  printf("%04d ", offset);

  OpCode opCode = chunk->code[offset];
  switch (opCode) {
    case OP_DEF_GLOBAL:
    case OP_GET_GLOBAL:
    case OP_SET_GLOBAL:
    case OP_CONSTANT:      return constant(chunk, offset);
    case OP_CALL:
    case OP_GET_LOCAL:
    case OP_SET_LOCAL:     return byte(chunk, offset);
    case OP_JUMP:
    case OP_JUMP_IF_TRUE:
    case OP_JUMP_IF_FALSE: return jump(chunk, 1, offset);
    case OP_LOOP:          return jump(chunk, -1, offset);
    case OP_ADD:
    case OP_SUBTRACT:
    case OP_MULTIPLY:
    case OP_DIVIDE:
    case OP_FALSE:
    case OP_TRUE:
    case OP_NOT:
    case OP_NEGATE:
    case OP_NIL:
    case OP_EQ:
    case OP_NOT_EQ:
    case OP_LESS:
    case OP_LESS_EQ:
    case OP_GREATER:
    case OP_GREATER_EQ:
    case OP_POP:
    case OP_PRINT:
    case OP_RETURN:        return single(chunk, offset);
  }

  printf("Unknown opcode %d\n", opCode);
  return offset + 1;
}

void disassembleChunk(Chunk *chunk, const char *name) {
  printf("== %s == \n", name);

  unsigned int offset = 0;
  while (offset < chunk->count) {
    offset = disassembleInstruction(chunk, offset);
  }
}
