#include "chunk.h"
#include "test_runners.h"

#include "minunit.h"

#define ASSERT_CHUNK_INIT(chunk)                         \
  do {                                                   \
    ASSERT_EQ_INT(0, chunk.capacity);                    \
    ASSERT_EQ_INT(0, chunk.count);                       \
    ASSERT_EQ_INT(true, chunk.code == NULL);             \
    ASSERT_EQ_INT(true, chunk.lines == NULL);            \
    ASSERT_EQ_INT(0, chunk.constants.count);             \
    ASSERT_EQ_INT(0, chunk.constants.capacity);          \
    ASSERT_EQ_INT(true, chunk.constants.values == NULL); \
  } while (0);

MU_TEST(test_initChunk) {
  Chunk chunk;

  initChunk(&chunk);

  ASSERT_CHUNK_INIT(chunk)

  freeChunk(&chunk);
}

MU_TEST(test_appendChunk) {
  Chunk chunk;
  initChunk(&chunk);

  appendChunk(&chunk, OP_RETURN, 1);

  ASSERT_EQ_INT(1, chunk.count);
  ASSERT_GE(chunk.capacity, 1, ""); // Assert capacity grew from reallocation
  ASSERT_EQ_INT(OP_RETURN, chunk.code[0]);

  freeChunk(&chunk);
}

MU_TEST(test_freeChunk) {
  Chunk chunk;
  initChunk(&chunk);
  appendChunk(&chunk, OP_RETURN, 1);

  freeChunk(&chunk);

  ASSERT_CHUNK_INIT(chunk)
}

MU_TEST(test_appendConstant) {
  Chunk chunk;
  initChunk(&chunk);
  Value constant = NUM_VAL(0.5);

  appendConstant(&chunk, constant);

  ASSERT_EQ_INT(1, chunk.constants.count);
  ASSERT_GE(chunk.constants.capacity, 1, "");
  ASSERT_EQ_INT(true, valuesEq(chunk.constants.values[0], constant));
}

MU_TEST_SUITE(chunk_tests) {
  MU_RUN_TEST(test_initChunk);
  MU_RUN_TEST(test_appendChunk);
  MU_RUN_TEST(test_freeChunk);
  MU_RUN_TEST(test_appendConstant);
}
