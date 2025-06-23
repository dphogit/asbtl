#include "compiler.h"

#include "chunk.h"
#include "test_runners.h"

#include "minunit.h"

// Asserts the expected bytecode array with the chunk's bytecode
static void assertBytecode(Chunk *chunk, uint8_t bytecode[], int n) {
  ASSERT_EQ_INT(n, chunk->count);

  for (int i = 0; i < n; i++) {
    ASSERT_EQ_INT(bytecode[i], chunk->code[i]);
  }
}

static void assertConstants(Chunk *chunk, Value constants[], int n) {
  ASSERT_EQ_INT(n, chunk->constants.count);

  for (int i = 0; i < n; i++) {
    ASSERT_EQ_INT(constants[i], chunk->constants.values[i]);
  }
}

MU_TEST_SUITE(test_compile_termExpression) {
  const char *source = "1 + 2 - 3";

  uint8_t expectedBytecode[] = {OP_CONSTANT, 0, OP_CONSTANT, 1,        OP_ADD,
                                OP_CONSTANT, 2, OP_SUBTRACT, OP_RETURN};
  Value expectedConstants[]  = {1, 2, 3};

  Chunk chunk;
  initChunk(&chunk);

  bool success = compile(source, &chunk);

  ASSERT_EQ_INT(true, success);
  assertBytecode(&chunk, expectedBytecode, sizeof(expectedBytecode));
  assertConstants(&chunk, expectedConstants, 3);

  freeChunk(&chunk);
}

MU_TEST_SUITE(test_compile_mixedPrecedence) {
  const char *source = "1 + 2 * 3";

  uint8_t expectedBytecode[] = {OP_CONSTANT, 0,           OP_CONSTANT,
                                1,           OP_CONSTANT, 2,
                                OP_MULTIPLY, OP_ADD,      OP_RETURN};
  Value expectedConstants[]  = {1, 2, 3};

  Chunk chunk;
  initChunk(&chunk);

  bool success = compile(source, &chunk);

  ASSERT_EQ_INT(true, success);
  assertBytecode(&chunk, expectedBytecode, sizeof(expectedBytecode));
  assertConstants(&chunk, expectedConstants, 3);

  freeChunk(&chunk);
}

MU_TEST_SUITE(compiler_tests) {
  MU_RUN_TEST(test_compile_termExpression);
  MU_RUN_TEST(test_compile_mixedPrecedence);
}
