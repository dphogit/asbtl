#include "compiler.h"

#include "chunk.h"
#include "test_runners.h"

#include "minunit.h"
#include "value.h"
#include "vm.h"

#define ASSERT_BYTECODE(chunk, bytecode, n) \
  ASSERT_EQ_INT(n, chunk.count);            \
  for (int i = 0; i < n; i++)               \
    ASSERT_EQ_INT(bytecode[i], chunk.code[i]);

#define ASSERT_CONSTS(chunk, consts, n)    \
  ASSERT_EQ_INT(n, chunk.constants.count); \
  for (int i = 0; i < n; i++)              \
    ASSERT_EQ_INT(true, valuesEq(consts[i], chunk.constants.values[i]));

Chunk chunk;

void test_setup(void) {
  initVM();
  initChunk(&chunk);
}

void test_teardown(void) {
  freeChunk(&chunk);
  freeVM();
}

MU_TEST(test_compile_termExpression) {
  const char *source = "1 + 2 - 3;";

  uint8_t expectedBytecode[] = {OP_CONSTANT, 0, OP_CONSTANT, 1,      OP_ADD,
                                OP_CONSTANT, 2, OP_SUBTRACT, OP_POP, OP_RETURN};

  Value expectedConstants[] = {
      {VAL_NUM, {.number = 1}},
      {VAL_NUM, {.number = 2}},
      {VAL_NUM, {.number = 3}}
  };

  bool success = compile(source, &chunk);

  ASSERT_EQ_INT(true, success);
  ASSERT_BYTECODE(chunk, expectedBytecode, 10);
  ASSERT_CONSTS(chunk, expectedConstants, 3);
}

MU_TEST(test_compile_mixedPrecedence) {
  const char *source = "1 + 2 * 3;";

  uint8_t expectedBytecode[] = {OP_CONSTANT, 0,        OP_CONSTANT, 1,
                                OP_CONSTANT, 2,        OP_MULTIPLY, OP_ADD,
                                OP_POP,      OP_RETURN};

  Value expectedConstants[] = {
      {VAL_NUM, {.number = 1}},
      {VAL_NUM, {.number = 2}},
      {VAL_NUM, {.number = 3}}
  };

  bool success = compile(source, &chunk);

  ASSERT_EQ_INT(true, success);
  ASSERT_BYTECODE(chunk, expectedBytecode, 10);
  ASSERT_CONSTS(chunk, expectedConstants, 3);
}

MU_TEST(test_compile_logicalAnd) {
  const char *source = "true && false;";

  uint8_t bytecode[] = {OP_TRUE, OP_JUMP_IF_FALSE, 0x00,   0x02,
                        OP_POP,  OP_FALSE,         OP_POP, OP_RETURN};

  bool success = compile(source, &chunk);

  ASSERT_EQ_INT(true, success);
  ASSERT_BYTECODE(chunk, bytecode, 8);
}

MU_TEST(test_compile_logicalOr) {
  const char *source = "true || false;";

  uint8_t bytecode[] = {OP_TRUE, OP_JUMP_IF_TRUE, 0x00,   0x02,
                        OP_POP,  OP_FALSE,        OP_POP, OP_RETURN};

  bool success = compile(source, &chunk);

  ASSERT_EQ_INT(true, success);
  ASSERT_BYTECODE(chunk, bytecode, 8);
}

MU_TEST(test_compile_ifStmt) {
  const char *source = "if (true) print true;";

  uint8_t bytecode[] = {OP_TRUE, OP_JUMP_IF_FALSE, 0x00,    0x06, OP_POP,
                        OP_TRUE, OP_PRINT,         OP_JUMP, 0x00, 0x01,
                        OP_POP,  OP_RETURN};

  bool success = compile(source, &chunk);

  ASSERT_EQ_INT(true, success);
  ASSERT_BYTECODE(chunk, bytecode, 12);
}

MU_TEST(test_compile_ifElseStmt) {
  const char *source = "if (true) print true; else print false;";

  uint8_t bytecode[] = {OP_TRUE, OP_JUMP_IF_FALSE, 0x00,     0x06,     OP_POP,
                        OP_TRUE, OP_PRINT,         OP_JUMP,  0x00,     0x03,
                        OP_POP,  OP_FALSE,         OP_PRINT, OP_RETURN};

  bool success = compile(source, &chunk);

  ASSERT_EQ_INT(true, success);
  ASSERT_BYTECODE(chunk, bytecode, 14);
}

MU_TEST(test_compile_whileLoop) {
  const char *source = "while (true) print true;";

  uint8_t bytecode[] = {OP_TRUE, OP_JUMP_IF_FALSE, 0x00,    0x06, OP_POP,
                        OP_TRUE, OP_PRINT,         OP_LOOP, 0x00, 0x0A,
                        OP_POP,  OP_RETURN};

  bool success = compile(source, &chunk);

  ASSERT_EQ_INT(true, success);
  ASSERT_BYTECODE(chunk, bytecode, 12);
}

MU_TEST(test_compile_forLoop_noClauses) {
  const char *source = "for (;;) print true;";

  uint8_t bytecode[] = {OP_TRUE, OP_PRINT, OP_LOOP, 0x00, 0x05, OP_RETURN};

  bool success = compile(source, &chunk);

  ASSERT_EQ_INT(true, success);
  ASSERT_BYTECODE(chunk, bytecode, 6);
}

MU_TEST(test_compile_forLoop_initializerOnly) {
  const char *source = "for (i = 0; ;) print true;";

  uint8_t bytecode[] = {OP_CONSTANT, 0x00,    OP_SET_GLOBAL, 0x01,
                        OP_POP,      OP_TRUE, OP_PRINT,      OP_LOOP,
                        0x00,        0x05,    OP_RETURN};
  Value constants[]  = {NUM_VAL(0), OBJ_VAL(copyString("i", 1))};

  bool success = compile(source, &chunk);

  ASSERT_EQ_INT(true, success);
  ASSERT_BYTECODE(chunk, bytecode, 11);
  ASSERT_CONSTS(chunk, constants, 2);
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

                        OP_POP, OP_RETURN};

  Value constants[] = {NUM_VAL(0), OBJ_VAL(copyString("i", 1)),
                       OBJ_VAL(copyString("i", 1)), NUM_VAL(5)};

  bool success = compile(source, &chunk);

  ASSERT_EQ_INT(true, success);
  ASSERT_BYTECODE(chunk, bytecode, 21);
  ASSERT_CONSTS(chunk, constants, 4);
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

                        OP_POP, OP_RETURN};

  Value i = OBJ_VAL(copyString("i", 1));

  Value constants[] = {NUM_VAL(0), i, i, NUM_VAL(5), i, NUM_VAL(1), i};

  bool success = compile(source, &chunk);

  ASSERT_EQ_INT(true, success);
  ASSERT_BYTECODE(chunk, bytecode, 35);
  ASSERT_CONSTS(chunk, constants, 7);
}

MU_TEST(test_compile_defineGlobalVariable) {
  const char *source = "var x = true;";

  uint8_t bytecode[] = {OP_TRUE, OP_DEF_GLOBAL, 0x00, OP_RETURN};
  Value constants[]  = {OBJ_VAL(copyString("x", 1))};

  bool success = compile(source, &chunk);

  ASSERT_EQ_INT(true, success);
  ASSERT_BYTECODE(chunk, bytecode, 4);
  ASSERT_CONSTS(chunk, constants, 1);
}

MU_TEST(test_compile_getGlobalVariable) {
  const char *source = "print x;";

  uint8_t bytecode[] = {OP_GET_GLOBAL, 0x00, OP_PRINT, OP_RETURN};
  Value constants[]  = {OBJ_VAL(copyString("x", 1))};

  bool success = compile(source, &chunk);

  ASSERT_EQ_INT(true, success);
  ASSERT_BYTECODE(chunk, bytecode, 4);
  ASSERT_CONSTS(chunk, constants, 1);
}

MU_TEST(test_compile_setGlobalVariable) {
  const char *source = "x = true;";

  uint8_t bytecode[] = {OP_TRUE, OP_SET_GLOBAL, 0x00, OP_POP, OP_RETURN};
  Value constants[]  = {OBJ_VAL(copyString("x", 1))};

  bool success = compile(source, &chunk);

  ASSERT_EQ_INT(true, success);
  ASSERT_BYTECODE(chunk, bytecode, 5);
  ASSERT_CONSTS(chunk, constants, 1);
}

MU_TEST_SUITE(compiler_tests) {
  MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

  MU_RUN_TEST(test_compile_termExpression);
  MU_RUN_TEST(test_compile_mixedPrecedence);

  MU_RUN_TEST(test_compile_logicalAnd);
  MU_RUN_TEST(test_compile_logicalOr);
  MU_RUN_TEST(test_compile_ifStmt);
  MU_RUN_TEST(test_compile_ifElseStmt);
  MU_RUN_TEST(test_compile_whileLoop);

  // Where I struggled! Had to take an `incremental` approach :)
  MU_RUN_TEST(test_compile_forLoop_noClauses);
  MU_RUN_TEST(test_compile_forLoop_initializerOnly);
  MU_RUN_TEST(test_compile_forLoop_initializerAndCondition);
  MU_RUN_TEST(test_compile_forLoop_allClauses);

  MU_RUN_TEST(test_compile_defineGlobalVariable);
  MU_RUN_TEST(test_compile_getGlobalVariable);
  MU_RUN_TEST(test_compile_setGlobalVariable);
}
