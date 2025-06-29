#ifndef ASBTL_VM_H
#define ASBTL_VM_H

#include "chunk.h"
#include "hashtable.h"
#include "value.h"

#include <stdint.h>

#define STACK_MAX 256

typedef struct vm {
  Value stack[STACK_MAX];
  Value *stackTop;
  Chunk *chunk;
  uint8_t *ip;       // Points to next bytecode instruction to execute
  Obj *objs;         // Intrusive linked list of runtime allocated objects
  HashTable strings; // String interning pool (hash set)
  HashTable globals; // Global variables
} VM;

extern VM vm;

void initVM();
void freeVM();

typedef enum interpret_result {
  INTERPRET_OK,
  INTERPRET_COMPILER_ERR,
  INTERPRET_RUNTIME_ERR
} InterpretResult;

InterpretResult interpret(const char *source);

#endif
