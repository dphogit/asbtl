#include "vm.h"

#include "chunk.h"
#include "compiler.h"
#include "hashtable.h"
#include "memory.h"
#include "object.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

VM vm;

#define TOP_CALLFRAME(vm) (&vm.frames[vm.frameCount - 1])

static void resetStack() {
  vm.stackTop     = vm.stack;
  vm.frameCount   = 0;
  vm.openUpvalues = NULL;
}

Value pop() {
  vm.stackTop--;
  return *vm.stackTop;
}

void push(Value value) {
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
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fprintf(stderr, "\n");

  for (int i = vm.frameCount - 1; i >= 0; i--) {
    CallFrame *frame = &vm.frames[i];
    ObjFunc *func    = frame->closure->func;
    size_t offset    = frame->ip - func->chunk.code - 1;

    fprintf(stderr, "[line %d] in ", func->chunk.lines[offset]);

    if (func->name == NULL) {
      fprintf(stderr, "script\n");
    } else {
      fprintf(stderr, "%s()\n", func->name->chars);
    }
  }

  resetStack();
}

static void defineNative(const char *name, NativeFn native) {
  // Push/pop off the name and function onto the stack to let the GC know that
  // we aren't done with these so the GC doesn't free them when a recollection
  // occurs dues to copyString and newNative dynamically allocating memory.

  push(OBJ_VAL(copyString(name, strlen(name))));
  push(OBJ_VAL(newNative(native)));

  hashTableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);

  pop();
  pop();
}

static Value clockNative(__attribute__((unused)) int argCount,
                         __attribute__((unused)) Value *args) {
  return NUM_VAL((double)clock() / CLOCKS_PER_SEC);
}

static void defineNativeFuncs() {
  defineNative("clock", clockNative);
}

static bool call(ObjClosure *closure, int argCount) {
  ObjFunc *func = closure->func;

  if (argCount != func->arity) {
    runtimeError("expected %d arguments, but got %d", func->arity, argCount);
    return false;
  }

  if (vm.frameCount == FRAMES_MAX) {
    runtimeError("stack overflow");
    return false;
  }

  CallFrame *frame = &vm.frames[vm.frameCount];
  vm.frameCount++;

  frame->closure = closure;
  frame->ip      = func->chunk.code;

  // slot 0 the compiler sets aside for class methods,
  // parameters start at slot 1 to align with arguments
  frame->slots = vm.stackTop - argCount - 1;

  return true;
}

static bool callValue(Value callee, int argCount) {
  if (IS_OBJ(callee)) {
    switch (OBJ_TYPE(callee)) {
      // case OBJ_FUNC:   return call(AS_FUNC(callee), argCount);
      case OBJ_CLOSURE: return call(AS_CLOSURE(callee), argCount);
      case OBJ_NATIVE:  {
        NativeFn native = AS_NATIVE(callee);
        Value *args     = vm.stackTop - argCount;
        Value result    = native(argCount, args);

        // Remove args from stack and push the result
        vm.stackTop -= argCount + 1;
        push(result);

        return true;
      }
      default: break;
    }
  }

  runtimeError("can only call functions");
  return false;
}

static ObjUpvalue *captureUpvalue(Value *local) {
  // Look for an existing upvalue before creating a new one by walking the
  // intrusive list of open upvalues (starting from closest to top of stack).
  ObjUpvalue *prev = NULL, *cur = vm.openUpvalues;
  while (cur != NULL && cur->location > local) {
    prev = cur;
    cur  = cur->next;
  }

  // Found an exising upvalue capturing the variable, reuse it.
  if (cur != NULL && cur->location == local) {
    return cur;
  }

  // Otherwise, create a new upvalue for the local slot and insert it at the
  // right location within the sorted list.
  ObjUpvalue *createdUpvalue = newUpvalue(local);
  createdUpvalue->next       = cur;

  if (prev == NULL) {
    vm.openUpvalues = createdUpvalue;
  } else {
    prev->next = createdUpvalue;
  }

  return createdUpvalue;
}

// Close every open upvalue pointing to the given stack slot or above it
static void closeUpvalues(Value *lastSlot) {
  while (vm.openUpvalues != NULL && vm.openUpvalues->location >= lastSlot) {
    ObjUpvalue *upvalue = vm.openUpvalues;

    // Closing an upvalue:
    // 1. Copy the variable's value into the `closed` field. This is where they
    //    live on the heap.
    // 2. OP_GET_UPVALUE/OP_SET_UPVALUE dereferences the `location` pointer to
    //    get the value, so we simply update the pointer to the `closed` field.
    upvalue->closed   = *upvalue->location;
    upvalue->location = &upvalue->closed;

    vm.openUpvalues = upvalue->next;
  }
}

void initVM() {
  resetStack();

  vm.objs           = NULL;
  vm.bytesAllocated = 0;
  vm.nextGC         = 1024 * 1024;
  vm.grayCapacity   = 0;
  vm.grayCount      = 0;
  vm.grayStack      = NULL;

  initHashTable(&vm.globals);
  initHashTable(&vm.strings);
  defineNativeFuncs();
}

void freeVM() {
  freeHashTable(&vm.strings);
  freeHashTable(&vm.globals);
  freeObjs();
}

// Heartbeat of the VM
static InterpretResult run() {
  CallFrame *frame = TOP_CALLFRAME(vm);

#define READ_BYTE()  (*frame->ip++)

#define READ_SHORT() (frame->ip += 2, ((frame->ip[-2] << 8) | frame->ip[-1]))

#define READ_CONSTANT() \
  (frame->closure->func->chunk.constants.values[READ_BYTE()])

#define READ_STRING() AS_STRING(READ_CONSTANT())

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
      case OP_ADD: {
        if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
          ObjString *b = AS_STRING(peek(0)), *a = AS_STRING(peek(1));
          Value result = OBJ_VAL(concatenate(a, b));
          pop();
          pop();
          push(result);
          continue;
        }

        if (IS_NUM(peek(0)) && IS_NUM(peek(1))) {
          double b = AS_NUM(pop()), a = AS_NUM(pop());
          push(NUM_VAL(a + b));
          continue;
        }

        runtimeError("operands must both be strings or both be numbers");
        return INTERPRET_RUNTIME_ERR;
      }
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
      case OP_NIL: push(NIL_VAL); continue;
      case OP_EQ:  {
        Value b = pop(), a = pop();
        push(BOOL_VAL(valuesEq(a, b)));
        continue;
      }
      case OP_NOT_EQ: {
        Value b = pop(), a = pop();
        push(BOOL_VAL(!valuesEq(a, b)));
        continue;
      }
      case OP_LESS:          BINARY_OP(BOOL_VAL, <); continue;
      case OP_LESS_EQ:       BINARY_OP(BOOL_VAL, <=); continue;
      case OP_GREATER:       BINARY_OP(BOOL_VAL, >); continue;
      case OP_GREATER_EQ:    BINARY_OP(BOOL_VAL, >=); continue;
      case OP_CONSTANT:      push(READ_CONSTANT()); continue;
      case OP_POP:           pop(); continue;
      case OP_JUMP_IF_FALSE: {
        uint16_t toJump = READ_SHORT();
        if (isFalsy(peek(0)))
          frame->ip += toJump;
        continue;
      }
      case OP_JUMP_IF_TRUE: {
        uint16_t toJump = READ_SHORT();
        if (!(isFalsy(peek(0))))
          frame->ip += toJump;
        continue;
      }
      case OP_JUMP: {
        uint16_t toJump = READ_SHORT();
        frame->ip += toJump;
        continue;
      }
      case OP_LOOP: {
        uint16_t toJumpBack = READ_SHORT();
        frame->ip -= toJumpBack;
        continue;
      }
      case OP_DEF_GLOBAL: {
        ObjString *name = READ_STRING();
        hashTableSet(&vm.globals, name, peek(0));
        pop();
        continue;
      }
      case OP_GET_GLOBAL: {
        ObjString *name = READ_STRING();
        Value value;
        if (!hashTableGet(&vm.globals, name, &value)) {
          runtimeError("undefined variable '%s'", name->chars);
          return INTERPRET_RUNTIME_ERR;
        }
        push(value);
        continue;
      }
      case OP_SET_GLOBAL: {
        ObjString *name = READ_STRING();
        if (hashTableSet(&vm.globals, name, peek(0))) {
          hashTableRemove(&vm.globals, name);
          runtimeError("undefined variable '%s'", name->chars);
          return INTERPRET_RUNTIME_ERR;
        }
        continue;
      }
      case OP_GET_LOCAL: {
        // Push the local's value onto the stack because other instructions
        // look for data at the top of the stack (therefore not redundant).
        uint8_t stackSlot = READ_BYTE();
        push(frame->slots[stackSlot]);
        continue;
      }
      case OP_SET_LOCAL: {
        // Assignment is an expression, so leave the value on the stack's top.
        uint8_t stackSlot       = READ_BYTE();
        frame->slots[stackSlot] = peek(0);
        continue;
      }
      case OP_GET_UPVALUE: {
        uint8_t slot = READ_BYTE();
        push(*frame->closure->upvalues[slot]->location);
        continue;
      }
      case OP_SET_UPVALUE: {
        uint8_t slot                              = READ_BYTE();
        *frame->closure->upvalues[slot]->location = peek(0);
        continue;
      }
      case OP_CLOSE_UPVALUE: {
        // The hoisted variable is at the stack's top.
        // Close/manage the upvalues and then discard the stack slot.
        closeUpvalues(vm.stackTop - 1);
        pop();
        continue;
      }
      case OP_CALL: {
        int argCount = READ_BYTE();
        Value value  = peek(argCount);

        if (!callValue(value, argCount)) {
          return INTERPRET_RUNTIME_ERR;
        }

        // callValue adds a new frame onto the call stack. Need to update so
        // the VM's next instruction executes the IP at the new function frame.
        frame = TOP_CALLFRAME(vm);
        continue;
      }
      case OP_CLOSURE: {
        ObjFunc *func       = AS_FUNC(READ_CONSTANT());
        ObjClosure *closure = newClosure(func);
        push(OBJ_VAL(closure));

        // Iterate over the pair of operands for each upvalue and capture it
        // from the enclosing function or a higher surrounding function
        // depending on the isLocal operand byte of the upvalue.
        for (int i = 0; i < closure->upvalueCount; i++) {
          uint8_t isLocal = READ_BYTE();
          uint8_t index   = READ_BYTE();

          // Capturing upvalues from the enclosing function (isLocal = false):
          // An OP_CLOSURE is emitted at the end of a function declaration, so
          // the moment we are executing that declaration, the current function
          // is the enclosing one - which is stored in the call frame at the
          // top of the callstack. So, to grab an upvalue from the enclosing
          // function we can directly read it from the the `frame` variable.
          closure->upvalues[i] = isLocal ? captureUpvalue(frame->slots + index)
                                         : frame->closure->upvalues[index];
        }

        continue;
      }
      case OP_PRINT: {
        printValue(pop());
        printf("\n");
        continue;
      }
      case OP_RETURN: {
        Value returnValue = pop();

        closeUpvalues(frame->slots);

        // Discard callframe
        vm.frameCount--;

        // If no frames left, have finished executing top-level code.
        if (vm.frameCount == 0) {
          pop();
          return INTERPRET_OK;
        }

        // Discard the slots the callee was using so the stack top ends up
        // pointing to the beginning of the returning functions stack window.
        vm.stackTop = frame->slots;

        // Then push the return value back onto the stack and update the pointed
        // to frame so the VM's execution resumes back at the called location.
        push(returnValue);
        frame = TOP_CALLFRAME(vm);
        continue;
      }
    }

    fprintf(stderr, "unknown instruction");
    return INTERPRET_RUNTIME_ERR;
  }

#undef BINARY_OP
#undef READ_CONSTANT
#undef READ_BYTE
}

InterpretResult interpret(const char *source) {
  // Compile and get the top level code - like a "main" function.
  ObjFunc *func = compile(source);
  if (func == NULL)
    return INTERPRET_COMPILER_ERR;

  // Store main function closure on stack, it's stack window starts at the
  // bottom of the VM's stack. We push/pop the func before/after the closure
  // creation for the GC to be aware of the heap-allocated object ObjFunc.
  push(OBJ_VAL(func));
  ObjClosure *closure = newClosure(func);
  pop();
  push(OBJ_VAL(closure));
  call(closure, 0);

  return run();
}
