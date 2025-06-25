#include "minunit.h"
#include "object.h"
#include "vm.h"

MU_TEST(test_initVM) {
  initVM();

  ASSERT_EQ_INT(true, vm.stackTop == vm.stack);
  ASSERT_EQ_INT(true, vm.objs == NULL);
}

MU_TEST(test_freeVM) {
  initVM();
  copyString("test", 4);

  freeVM();

  ASSERT_EQ_INT(true, vm.objs == NULL);
}

MU_TEST_SUITE(vm_tests) {
  MU_RUN_TEST(test_initVM);
  MU_RUN_TEST(test_freeVM);
}
