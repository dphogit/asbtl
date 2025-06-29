#include "compiler.h"

#include "chunk.h"
#include "test_runners.h"

#include "minunit.h"
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
  MU_RUN_TEST(test_compile_defineGlobalVariable);
  MU_RUN_TEST(test_compile_getGlobalVariable);
  MU_RUN_TEST(test_compile_setGlobalVariable);
}
