#include "vm.h"

#include "chunk.h"
#include "compiler.h"

#include <stdarg.h>
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

static Value peek(int dist) {
  return vm.stackTop[-(dist + 1)];
}

static void runtimeError(const char *format, ...) {
  int offset = vm.ip - vm.chunk->code - 1;
  int line   = vm.chunk->lines[offset];
  fprintf(stderr, "[line %d] runtime error: ", line);

  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fprintf(stderr, "\n");

  resetStack();
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
#define BINARY_OP(op)                          \
  if (!IS_NUM(peek(0)) || !IS_NUM(peek(1))) {  \
    runtimeError("operands must be numbers");  \
    return INTERPRET_RUNTIME_ERR;              \
  }                                            \
  double b = AS_NUM(pop()), a = AS_NUM(pop()); \
  push(NUM_VAL(a op b));

  while (true) {
    OpCode opCode = READ_BYTE();

    switch (opCode) {
      case OP_ADD: {
        BINARY_OP(+);
        break;
      }
      case OP_SUBTRACT: {
        BINARY_OP(-);
        break;
      }
      case OP_MULTIPLY: {
        BINARY_OP(*);
        break;
      }
      case OP_DIVIDE: {
        BINARY_OP(/);
        break;
      }
      case OP_TRUE: {
        push(BOOL_VAL(true));
        break;
      }
      case OP_FALSE: {
        push(BOOL_VAL(false));
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

#undef BINARY_OP
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
