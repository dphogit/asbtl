#include "chunk.h"
#include "test_runners.h"

#include "minunit.h"

#define ASSERT_CHUNK_INIT(chunk)           \
  do {                                     \
    ASSERT_EQ_INT(0, chunk.capacity);      \
    ASSERT_EQ_INT(0, chunk.count);         \
    mu_check(chunk.code == NULL, "");      \
    mu_check(chunk.constants == NULL, ""); \
  } while (0);

MU_TEST(test_initChunk) {
  Chunk chunk;

  initChunk(&chunk);

  ASSERT_CHUNK_INIT(chunk)
}

MU_TEST(test_appendChunk) {
  Chunk chunk;
  initChunk(&chunk);

  appendChunk(&chunk, OP_RETURN);

  ASSERT_EQ_INT(1, chunk.count);
  ASSERT_GE(chunk.capacity, 1, ""); // Assert capacity grew from reallocation
  ASSERT_EQ_INT(OP_RETURN, chunk.code[0]);
}

MU_TEST(test_freeChunk) {
  Chunk chunk;
  initChunk(&chunk);
  appendChunk(&chunk, OP_RETURN);

  freeChunk(&chunk);

  ASSERT_CHUNK_INIT(chunk)
}

MU_TEST_SUITE(chunk_tests) {
  MU_RUN_TEST(test_initChunk);
  MU_RUN_TEST(test_appendChunk);
  MU_RUN_TEST(test_freeChunk);
}
