#include "compiler.h"

#include "chunk.h"
#include "test_runners.h"

#include "minunit.h"

#define ASSERT_BYTECODE(chunk, bytecode, n) \
  ASSERT_EQ_INT(n, chunk.count);            \
  for (int i = 0; i < n; i++)               \
    ASSERT_EQ_INT(bytecode[i], chunk.code[i]);

#define ASSERT_CONSTS(chunk, consts, n)    \
  ASSERT_EQ_INT(n, chunk.constants.count); \
  for (int i = 0; i < n; i++)              \
    ASSERT_EQ_INT(true, valuesEq(consts[i], chunk.constants.values[i]));

MU_TEST_SUITE(test_compile_termExpression) {
  const char *source = "1 + 2 - 3";

  uint8_t expectedBytecode[] = {OP_CONSTANT, 0, OP_CONSTANT, 1,        OP_ADD,
                                OP_CONSTANT, 2, OP_SUBTRACT, OP_RETURN};

  Value expectedConstants[] = {
      {VAL_NUM, {.number = 1}},
      {VAL_NUM, {.number = 2}},
      {VAL_NUM, {.number = 3}}
  };

  Chunk chunk;
  initChunk(&chunk);

  bool success = compile(source, &chunk);

  ASSERT_EQ_INT(true, success);
  ASSERT_BYTECODE(chunk, expectedBytecode, 9);
  ASSERT_CONSTS(chunk, expectedConstants, 3);

  freeChunk(&chunk);
}

MU_TEST_SUITE(test_compile_mixedPrecedence) {
  const char *source = "1 + 2 * 3";

  uint8_t expectedBytecode[] = {OP_CONSTANT, 0,           OP_CONSTANT,
                                1,           OP_CONSTANT, 2,
                                OP_MULTIPLY, OP_ADD,      OP_RETURN};

  Value expectedConstants[] = {
      {VAL_NUM, {.number = 1}},
      {VAL_NUM, {.number = 2}},
      {VAL_NUM, {.number = 3}}
  };

  Chunk chunk;
  initChunk(&chunk);

  bool success = compile(source, &chunk);

  ASSERT_EQ_INT(true, success);
  ASSERT_BYTECODE(chunk, expectedBytecode, 9);
  ASSERT_CONSTS(chunk, expectedConstants, 3);

  freeChunk(&chunk);
}

MU_TEST_SUITE(compiler_tests) {
  MU_RUN_TEST(test_compile_termExpression);
  MU_RUN_TEST(test_compile_mixedPrecedence);
}
