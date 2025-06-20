#include "compiler.h"

#include "chunk.h"
#include "test_runners.h"

#include "minunit.h"

MU_TEST_SUITE(test_compile_termExpression) {
  const char *source = "1 + 2 - 3";

  Chunk chunk;
  initChunk(&chunk);

  bool success = compile(source, &chunk);

  ASSERT_EQ_INT(true, success);
  ASSERT_EQ_INT(9, chunk.count);
  ASSERT_EQ_INT(3, chunk.constants.count);

  ASSERT_EQ_INT(OP_CONSTANT, chunk.code[0]);
  ASSERT_EQ_INT(1, chunk.constants.values[chunk.code[1]]);

  ASSERT_EQ_INT(OP_CONSTANT, chunk.code[2]);
  ASSERT_EQ_INT(2, chunk.constants.values[chunk.code[3]]);

  ASSERT_EQ_INT(OP_ADD, chunk.code[4]);

  ASSERT_EQ_INT(OP_CONSTANT, chunk.code[5]);
  ASSERT_EQ_INT(3, chunk.constants.values[chunk.code[6]]);

  ASSERT_EQ_INT(OP_SUBTRACT, chunk.code[7]);

  ASSERT_EQ_INT(OP_RETURN, chunk.code[8]);
}

MU_TEST_SUITE(compiler_tests) {
  MU_RUN_TEST(test_compile_termExpression);
}
