#include "compiler.h"

#include "chunk.h"
#include "object.h"
#include "test_runners.h"

#include "minunit.h"
#include "value.h"
#include "vm.h"

#define ASSERT_NOT_NULL(x) ASSERT_EQ_INT(true, x != NULL)

#define ASSERT_BYTECODE(chunk, bytecode, n) \
  ASSERT_EQ_INT(n, chunk.count);            \
  for (int i = 0; i < n; i++)               \
    ASSERT_EQ_INT(bytecode[i], chunk.code[i]);

#define ASSERT_CONSTS(chunk, consts, n)    \
  ASSERT_EQ_INT(n, chunk.constants.count); \
  for (int i = 0; i < n; i++)              \
    ASSERT_EQ_INT(true, valuesEq(consts[i], chunk.constants.values[i]));

void test_setup(void) {
  initVM();
}

void test_teardown(void) {
  freeVM();
}

MU_TEST(test_compile_termExpression) {
  const char *source = "1 + 2 - 3;";

  uint8_t expectedBytecode[] = {OP_CONSTANT, 0, OP_CONSTANT, 1,      OP_ADD,
                                OP_CONSTANT, 2, OP_SUBTRACT, OP_POP, OP_NIL,
                                OP_RETURN};

  Value expectedConstants[] = {
      {VAL_NUM, {.number = 1}},
      {VAL_NUM, {.number = 2}},
      {VAL_NUM, {.number = 3}}
  };

  ObjFunc *func = compile(source);

  ASSERT_NOT_NULL(func);
  ASSERT_BYTECODE(func->chunk, expectedBytecode, 11);
  ASSERT_CONSTS(func->chunk, expectedConstants, 3);
}

MU_TEST(test_compile_mixedPrecedence) {
  const char *source = "1 + 2 * 3;";

  uint8_t expectedBytecode[] = {OP_CONSTANT, 0,      OP_CONSTANT, 1,
                                OP_CONSTANT, 2,      OP_MULTIPLY, OP_ADD,
                                OP_POP,      OP_NIL, OP_RETURN};

  Value expectedConstants[] = {
      {VAL_NUM, {.number = 1}},
      {VAL_NUM, {.number = 2}},
      {VAL_NUM, {.number = 3}}
  };

  ObjFunc *func = compile(source);

  ASSERT_NOT_NULL(func);
  ASSERT_BYTECODE(func->chunk, expectedBytecode, 11);
  ASSERT_CONSTS(func->chunk, expectedConstants, 3);
}

MU_TEST(test_compile_logicalAnd) {
  const char *source = "true && false;";

  uint8_t bytecode[] = {OP_TRUE,  OP_JUMP_IF_FALSE, 0x00,   0x02,     OP_POP,
                        OP_FALSE, OP_POP,           OP_NIL, OP_RETURN};

  ObjFunc *func = compile(source);

  ASSERT_NOT_NULL(func);
  ASSERT_BYTECODE(func->chunk, bytecode, 9);
}

MU_TEST(test_compile_logicalOr) {
  const char *source = "true || false;";

  uint8_t bytecode[] = {OP_TRUE,  OP_JUMP_IF_TRUE, 0x00,   0x02,     OP_POP,
                        OP_FALSE, OP_POP,          OP_NIL, OP_RETURN};

  ObjFunc *func = compile(source);

  ASSERT_NOT_NULL(func);
  ASSERT_BYTECODE(func->chunk, bytecode, 9);
}

MU_TEST(test_compile_ifStmt) {
  const char *source = "if (true) print true;";

  uint8_t bytecode[] = {OP_TRUE, OP_JUMP_IF_FALSE, 0x00,     0x06, OP_POP,
                        OP_TRUE, OP_PRINT,         OP_JUMP,  0x00, 0x01,
                        OP_POP,  OP_NIL,           OP_RETURN};

  ObjFunc *func = compile(source);

  ASSERT_NOT_NULL(func);
  ASSERT_BYTECODE(func->chunk, bytecode, 13);
}

MU_TEST(test_compile_ifElseStmt) {
  const char *source = "if (true) print true; else print false;";

  uint8_t bytecode[] = {OP_TRUE, OP_JUMP_IF_FALSE, 0x00,     0x06,   OP_POP,
                        OP_TRUE, OP_PRINT,         OP_JUMP,  0x00,   0x03,
                        OP_POP,  OP_FALSE,         OP_PRINT, OP_NIL, OP_RETURN};

  ObjFunc *func = compile(source);

  ASSERT_NOT_NULL(func);
  ASSERT_BYTECODE(func->chunk, bytecode, 15);
}

MU_TEST(test_compile_conditional) {
  const char *source = "print true ? true : false;";

  uint8_t bytecode[] = {OP_TRUE,  OP_JUMP_IF_FALSE, 0x00,   0x05,     OP_POP,
                        OP_TRUE,  OP_JUMP,          0x00,   0x02,     OP_POP,
                        OP_FALSE, OP_PRINT,         OP_NIL, OP_RETURN};

  ObjFunc *func = compile(source);

  ASSERT_NOT_NULL(func);
  ASSERT_BYTECODE(func->chunk, bytecode, 14);
}

MU_TEST(test_compile_whileLoop) {
  const char *source = "while (true) print true;";

  uint8_t bytecode[] = {OP_TRUE, OP_JUMP_IF_FALSE, 0x00,     0x06, OP_POP,
                        OP_TRUE, OP_PRINT,         OP_LOOP,  0x00, 0x0A,
                        OP_POP,  OP_NIL,           OP_RETURN};

  ObjFunc *func = compile(source);

  ASSERT_NOT_NULL(func);
  ASSERT_BYTECODE(func->chunk, bytecode, 13);
}

MU_TEST(test_compile_forLoop_noClauses) {
  const char *source = "for (;;) print true;";

  uint8_t bytecode[] = {OP_TRUE, OP_PRINT, OP_LOOP,  0x00,
                        0x05,    OP_NIL,   OP_RETURN};

  ObjFunc *func = compile(source);

  ASSERT_NOT_NULL(func);
  ASSERT_BYTECODE(func->chunk, bytecode, 7);
}

MU_TEST(test_compile_forLoop_initializerOnly) {
  const char *source = "for (i = 0; ;) print true;";

  uint8_t bytecode[] = {OP_CONSTANT, 0x00,    OP_SET_GLOBAL, 0x01,
                        OP_POP,      OP_TRUE, OP_PRINT,      OP_LOOP,
                        0x00,        0x05,    OP_NIL,        OP_RETURN};
  Value constants[]  = {NUM_VAL(0), OBJ_VAL(copyString("i", 1))};

  ObjFunc *func = compile(source);

  ASSERT_NOT_NULL(func);
  ASSERT_BYTECODE(func->chunk, bytecode, 12);
  ASSERT_CONSTS(func->chunk, constants, 2);
}

MU_TEST(test_compile_forLoop_initializerAndCondition) {
  const char *source = "for (i = 0; i < 5; ) print true;";

  uint8_t bytecode[] = {// Initializer
                        OP_CONSTANT, 0x00, OP_SET_GLOBAL, 0x01, OP_POP,
                        // Initializer end

                        // Condition
                        OP_GET_GLOBAL, 0x02, OP_CONSTANT, 0x03, OP_LESS,
                        OP_JUMP_IF_FALSE, 0x00, 0x06, OP_POP,
                        // Condition end

                        // Body
                        OP_TRUE, OP_PRINT, OP_LOOP, 0x00, 0x0E,
                        // Body end

                        OP_POP, OP_NIL, OP_RETURN};

  Value constants[] = {NUM_VAL(0), OBJ_VAL(copyString("i", 1)),
                       OBJ_VAL(copyString("i", 1)), NUM_VAL(5)};

  ObjFunc *func = compile(source);

  ASSERT_NOT_NULL(func);
  ASSERT_BYTECODE(func->chunk, bytecode, 22);
  ASSERT_CONSTS(func->chunk, constants, 4);
}

MU_TEST(test_compile_forLoop_allClauses) {
  const char *source = "for (i = 0; i < 5; i = i + 1) print true;";

  uint8_t bytecode[] = {// Initializer start
                        OP_CONSTANT, 0x00, OP_SET_GLOBAL, 0x01, OP_POP,
                        // Initializer end

                        // Condition start
                        OP_GET_GLOBAL, 0x02, OP_CONSTANT, 0x03, OP_LESS,
                        OP_JUMP_IF_FALSE, 0x00, 0x14, OP_POP, OP_JUMP, 0x00,
                        0x0B,
                        // Condition end

                        // Increment start
                        OP_GET_GLOBAL, 0x04, OP_CONSTANT, 0x05, OP_ADD,
                        OP_SET_GLOBAL, 0x06, OP_POP, OP_LOOP, 0x00, 0x17,
                        // Increment end - jump back to condition

                        // Body start
                        OP_TRUE, OP_PRINT, OP_LOOP, 0x00, 0x10,
                        // Body end - jump to increment start

                        OP_POP, OP_NIL, OP_RETURN};

  Value i = OBJ_VAL(copyString("i", 1));

  Value constants[] = {NUM_VAL(0), i, i, NUM_VAL(5), i, NUM_VAL(1), i};

  ObjFunc *func = compile(source);

  ASSERT_NOT_NULL(func);
  ASSERT_BYTECODE(func->chunk, bytecode, 36);
  ASSERT_CONSTS(func->chunk, constants, 7);
}

MU_TEST(test_compile_defineGlobalVariable) {
  const char *source = "var x = true;";

  uint8_t bytecode[] = {OP_TRUE, OP_DEF_GLOBAL, 0x00, OP_NIL, OP_RETURN};
  Value constants[]  = {OBJ_VAL(copyString("x", 1))};

  ObjFunc *func = compile(source);

  ASSERT_NOT_NULL(func);
  ASSERT_BYTECODE(func->chunk, bytecode, 5);
  ASSERT_CONSTS(func->chunk, constants, 1);
}

MU_TEST(test_compile_getGlobalVariable) {
  const char *source = "print x;";

  uint8_t bytecode[] = {OP_GET_GLOBAL, 0x00, OP_PRINT, OP_NIL, OP_RETURN};
  Value constants[]  = {OBJ_VAL(copyString("x", 1))};

  ObjFunc *func = compile(source);

  ASSERT_NOT_NULL(func);
  ASSERT_BYTECODE(func->chunk, bytecode, 5);
  ASSERT_CONSTS(func->chunk, constants, 1);
}

MU_TEST(test_compile_setGlobalVariable) {
  const char *source = "x = true;";

  uint8_t bytecode[] = {OP_TRUE, OP_SET_GLOBAL, 0x00,
                        OP_POP,  OP_NIL,        OP_RETURN};
  Value constants[]  = {OBJ_VAL(copyString("x", 1))};

  ObjFunc *func = compile(source);

  ASSERT_NOT_NULL(func);
  ASSERT_BYTECODE(func->chunk, bytecode, 6);
  ASSERT_CONSTS(func->chunk, constants, 1);
}

MU_TEST(test_compile_localVariable) {
  const char *source = "{ var x; x = true; print x; }";

  // First value on stack (0x00) is the "main" function value,
  // so the local variable is stored on the next stack slot (at 0x01).
  uint8_t bytecode[] = {OP_NIL, OP_TRUE,      OP_SET_LOCAL, 0x01,
                        OP_POP, OP_GET_LOCAL, 0x01,         OP_PRINT,
                        OP_POP, OP_NIL,       OP_RETURN};

  ObjFunc *func = compile(source);

  ASSERT_NOT_NULL(func);
  ASSERT_BYTECODE(func->chunk, bytecode, 11);
}

MU_TEST(test_compile_localVariable_initializer) {
  const char *source = "{ var x = true; }";

  uint8_t bytecode[] = {OP_TRUE, OP_POP, OP_NIL, OP_RETURN};

  ObjFunc *func = compile(source);

  ASSERT_NOT_NULL(func);
  ASSERT_BYTECODE(func->chunk, bytecode, 4);
}

MU_TEST(test_compile_function_noParams) {
  const char *source = "func printTrue() { print true; }";

  uint8_t outerBytecode[] = {OP_CLOSURE, 0x01,   OP_DEF_GLOBAL,
                             0x00,       OP_NIL, OP_RETURN};
  uint8_t innerBytecode[] = {OP_TRUE, OP_PRINT, OP_NIL, OP_RETURN};

  ObjFunc *mainFunc = compile(source);

  ASSERT_NOT_NULL(mainFunc);
  ASSERT_BYTECODE(mainFunc->chunk, outerBytecode, 6);

  ASSERT_EQ_INT(2, mainFunc->chunk.constants.count);
  ASSERT_EQ_INT(true, valuesEq(OBJ_VAL(copyString("printTrue", 9)),
                               mainFunc->chunk.constants.values[0]));

  ASSERT_EQ_INT(true, IS_FUNC(mainFunc->chunk.constants.values[1]));
  ObjFunc *innerFunc = AS_FUNC(mainFunc->chunk.constants.values[1]);

  ASSERT_EQ_INT(0, innerFunc->arity);
  ASSERT_STREQ("printTrue", innerFunc->name->chars);

  ASSERT_BYTECODE(innerFunc->chunk, innerBytecode, 4);
}

MU_TEST(test_compile_function_returnValue) {
  const char *source = "func returnTrue() { return true; }";

  // Compiler always emits OP_NIL, OP_RETURN at the end of each function
  // as a fallback in case there is no explicit return value. This doesn't
  // affect the VM's runtime execution because it is shortcutted by the
  // first OP_RETURN if there was an explicit return statement compiled.
  uint8_t funcBytecode[] = {OP_TRUE, OP_RETURN, OP_NIL, OP_RETURN};

  ObjFunc *mainFunc = compile(source);

  ObjFunc *innerFunc = AS_FUNC(mainFunc->chunk.constants.values[1]);
  ASSERT_EQ_INT(0, innerFunc->arity);
  ASSERT_STREQ("returnTrue", innerFunc->name->chars);
  ASSERT_BYTECODE(innerFunc->chunk, funcBytecode, 4);
}

MU_TEST(test_compile_function_simpleClosure) {
  const char *source = "{"
                       "  var a = 3;"
                       "  func f() {"
                       "    print a;"
                       "  }"
                       "}";

  // locals = ["", "a", "f"]
  // constants = [3, <fn f>]
  uint8_t mainBytecode[] = {
      OP_CONSTANT, 0x00,   OP_CLOSURE,       0x01,   UPVALUE_CAPTURES_LOCAL,
      0x01,        OP_POP, OP_CLOSE_UPVALUE, OP_NIL, OP_RETURN};

  // upvalues: [(local=true, i=1)]
  uint8_t fBytecode[] = {OP_GET_UPVALUE, 0x00, OP_PRINT, OP_NIL, OP_RETURN};

  ObjFunc *main = compile(source);

  ASSERT_NOT_NULL(main);
  ASSERT_BYTECODE(main->chunk, mainBytecode, 10);
  ASSERT_EQ_INT(2, main->chunk.constants.count);
  ASSERT_EQ_INT(true, valuesEq(NUM_VAL(3), main->chunk.constants.values[0]));
  ASSERT_EQ_INT(true, IS_FUNC(main->chunk.constants.values[1]));

  ObjFunc *f = AS_FUNC(main->chunk.constants.values[1]);

  ASSERT_BYTECODE(f->chunk, fBytecode, 5);
  ASSERT_EQ_INT(1, f->upvalueCount);
  ASSERT_STREQ("f", f->name->chars);
  ASSERT_EQ_INT(0, f->arity);
}

MU_TEST(test_compile_function_counterClosure) {
  const char *source = "func makeCounter() {"
                       "  var count = 0;"
                       "  func inc() { count = count + 1; print count; }"
                       "  return inc;"
                       "}";

  // constants = ["makeCounter", <fn makeCounter>]
  uint8_t mainBytecode[] = {OP_CLOSURE, 0x01,   OP_DEF_GLOBAL,
                            0x00,       OP_NIL, OP_RETURN};

  // locals = ["", "count", "inc"]
  // constants: [0, <fn inc>]
  uint8_t makeCounterBytecode[] = {
      OP_CONSTANT,  0x00, OP_CLOSURE, 0x01,   UPVALUE_CAPTURES_LOCAL, 0x01,
      OP_GET_LOCAL, 0x02, OP_RETURN,  OP_POP, OP_CLOSE_UPVALUE,       OP_NIL,
      OP_RETURN};

  // constants: [1]
  // upvalues = [(local=true, i=1)]
  uint8_t incBytecode[] = {OP_GET_UPVALUE, 0x00,           OP_CONSTANT, 0x00,
                           OP_ADD,         OP_SET_UPVALUE, 0x00,        OP_POP,
                           OP_GET_UPVALUE, 0x00,           OP_PRINT,    OP_NIL,
                           OP_RETURN};

  ObjFunc *main = compile(source);

  ASSERT_NOT_NULL(main);
  ASSERT_BYTECODE(main->chunk, mainBytecode, 6);
  ASSERT_EQ_INT(2, main->chunk.constants.count);
  ASSERT_EQ_INT(true, IS_STRING(main->chunk.constants.values[0]));
  ASSERT_STREQ("makeCounter", AS_CSTRING(main->chunk.constants.values[0]));
  ASSERT_EQ_INT(true, IS_FUNC(main->chunk.constants.values[1]));

  ObjFunc *makeCounter = AS_FUNC(main->chunk.constants.values[1]);
  ASSERT_STREQ("makeCounter", makeCounter->name->chars);
  ASSERT_EQ_INT(0, makeCounter->arity);

  Chunk counterChunk = makeCounter->chunk;
  ASSERT_BYTECODE(makeCounter->chunk, makeCounterBytecode, 13);
  ASSERT_EQ_INT(2, counterChunk.constants.count);
  ASSERT_EQ_INT(true, valuesEq(NUM_VAL(0), counterChunk.constants.values[0]));
  ASSERT_EQ_INT(true, IS_FUNC(counterChunk.constants.values[1]));

  ObjFunc *inc = AS_FUNC(counterChunk.constants.values[1]);
  ASSERT_STREQ("inc", inc->name->chars);
  ASSERT_EQ_INT(0, inc->arity);
  ASSERT_EQ_INT(inc->upvalueCount, 1);

  Chunk incChunk = inc->chunk;
  ASSERT_BYTECODE(inc->chunk, incBytecode, 13);
  ASSERT_EQ_INT(1, incChunk.constants.count);
  ASSERT_EQ_INT(true, valuesEq(NUM_VAL(1), incChunk.constants.values[0]));
}

MU_TEST_SUITE(compiler_tests) {
  MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

  MU_RUN_TEST(test_compile_termExpression);
  MU_RUN_TEST(test_compile_mixedPrecedence);

  MU_RUN_TEST(test_compile_logicalAnd);
  MU_RUN_TEST(test_compile_logicalOr);
  MU_RUN_TEST(test_compile_ifStmt);
  MU_RUN_TEST(test_compile_ifElseStmt);
  MU_RUN_TEST(test_compile_conditional);
  MU_RUN_TEST(test_compile_whileLoop);

  // 'for' loops were tough! Had to take an `incremental` approach :)
  MU_RUN_TEST(test_compile_forLoop_noClauses);
  MU_RUN_TEST(test_compile_forLoop_initializerOnly);
  MU_RUN_TEST(test_compile_forLoop_initializerAndCondition);
  MU_RUN_TEST(test_compile_forLoop_allClauses);

  MU_RUN_TEST(test_compile_defineGlobalVariable);
  MU_RUN_TEST(test_compile_getGlobalVariable);
  MU_RUN_TEST(test_compile_setGlobalVariable);
  MU_RUN_TEST(test_compile_localVariable);
  MU_RUN_TEST(test_compile_localVariable_initializer);

  MU_RUN_TEST(test_compile_function_noParams);
  MU_RUN_TEST(test_compile_function_returnValue);
  MU_RUN_TEST(test_compile_function_simpleClosure);
  MU_RUN_TEST(test_compile_function_counterClosure);
}
