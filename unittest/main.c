#include "test_runners.h"

#include "minunit.h"

int main(void) {
  MU_RUN_SUITE(chunk_tests, "Chunk Tests");
  MU_RUN_SUITE(scanner_tests, "Scanner Tests");
  MU_RUN_SUITE(value_tests, "Value Tests");

  MU_REPORT();
  return MU_EXIT_CODE;
}
