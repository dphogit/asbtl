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

// false and nil are "falsy", everything else is "truthy"
static bool isFalsy(Value value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
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
#define BINARY_OP(valueType, op)                 \
  do {                                           \
    if (!IS_NUM(peek(0)) || !IS_NUM(peek(1))) {  \
      runtimeError("operands must be numbers");  \
      return INTERPRET_RUNTIME_ERR;              \
    }                                            \
    double b = AS_NUM(pop()), a = AS_NUM(pop()); \
    push(valueType(a op b));                     \
  } while (false)

  while (true) {
    OpCode opCode = READ_BYTE();

    switch (opCode) {
      case OP_ADD:      BINARY_OP(NUM_VAL, +); continue;
      case OP_SUBTRACT: BINARY_OP(NUM_VAL, -); continue;
      case OP_MULTIPLY: BINARY_OP(NUM_VAL, *); continue;
      case OP_DIVIDE:   BINARY_OP(NUM_VAL, /); continue;
      case OP_FALSE:    push(BOOL_VAL(false)); continue;
      case OP_TRUE:     push(BOOL_VAL(true)); continue;
      case OP_NOT:      vm.stackTop[-1] = BOOL_VAL(isFalsy(peek(0))); continue;
      case OP_NEGATE:   {
        if (!IS_NUM(peek(0))) {
          runtimeError("negation operand must be a number");
          return INTERPRET_RUNTIME_ERR;
        }
        vm.stackTop[-1].as.number = -vm.stackTop[-1].as.number;
        continue;
      }
      case OP_NIL:        push(NIL_VAL()); continue;
      case OP_EQ:         BINARY_OP(BOOL_VAL, ==); continue;
      case OP_NOT_EQ:     BINARY_OP(BOOL_VAL, !=); continue;
      case OP_LESS:       BINARY_OP(BOOL_VAL, <); continue;
      case OP_LESS_EQ:    BINARY_OP(BOOL_VAL, <=); continue;
      case OP_GREATER:    BINARY_OP(BOOL_VAL, >); continue;
      case OP_GREATER_EQ: BINARY_OP(BOOL_VAL, >=); continue;
      case OP_CONSTANT:   push(READ_CONSTANT()); continue;
      case OP_RETURN:
        printValue(pop());
        printf("\n");
        return INTERPRET_OK;
    }

    fprintf(stderr, "unknown instruction");
    return INTERPRET_RUNTIME_ERR;
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
