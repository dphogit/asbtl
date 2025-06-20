#include "vm.h"

#include "chunk.h"
#include "compiler.h"

#include <stdio.h>

VM vm;

static void resetStack() {
  vm.stackTop = vm.stack;
}

static Value pop() {
  vm.stackTop--;
  return *vm.stackTop;
}

static void push(Value value) {
  *vm.stackTop = value;
  vm.stackTop++;
}

void initVM() {
  resetStack();
}

void freeVM() {
  // The objects the VM refers to will be freed here
}

static InterpretResult run() {
#define READ_BYTE()     (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

  while (true) {
    OpCode opCode = READ_BYTE();

    switch (opCode) {
      case OP_ADD: {
        Value b = pop();
        Value a = pop();
        push(a + b);
        break;
      }
      case OP_SUBTRACT: {
        Value b = pop();
        Value a = pop();
        push(a - b);
        break;
      }
      case OP_CONSTANT: {
        push(READ_CONSTANT());
        break;
      }
      case OP_RETURN: {
        printValue(pop());
        printf("\n");
        return INTERPRET_OK;
      }
      default:
        fprintf(stderr, "unknown instruction");
        return INTERPRET_RUNTIME_ERR;
    }
  }

#undef READ_CONSTANT
#undef READ_BYTE
}

InterpretResult interpret(const char *source) {
  Chunk chunk;
  initChunk(&chunk);

  if (!compile(source, &chunk)) {
    freeChunk(&chunk);
    return INTERPRET_COMPILER_ERR;
  }

  vm.chunk = &chunk;
  vm.ip    = chunk.code;

  InterpretResult result = run();

  freeChunk(&chunk);

  return result;
}
