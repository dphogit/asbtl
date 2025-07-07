#ifndef ASBTL_VM_H
#define ASBTL_VM_H

#include "hashtable.h"
#include "value.h"

#include <stdint.h>

#define FRAMES_MAX 64
#define STACK_MAX  (FRAMES_MAX * 256)

// Represents a function invocation
typedef struct call_frame {
  ObjFunc *func; // The function object being executed
  uint8_t *ip;   // Each frame has its own IP for next instruction to execute
  Value *slots;  // Points into the VM's value stack at the first slot it uses
} CallFrame;

typedef struct vm {
  CallFrame frames[FRAMES_MAX]; // The call frame stack
  int frameCount;               // Height of the call frame stack
  Value stack[STACK_MAX];
  Value *stackTop;
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
