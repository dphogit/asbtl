#include "minunit.h"
#include "object.h"
#include "test_runners.h"
#include "vm.h"

#include <stdbool.h>

void setup() {
  initVM();
}

void teardown() {
  freeVM();
}

MU_TEST(test_copyString) {
  ObjString *result = copyString("test", 4);

  ASSERT_EQ_INT(true, result != NULL);
  ASSERT_STREQ("test", result->chars);
  ASSERT_EQ_INT(4, result->len);

  ASSERT_EQ_INT(true, vm.objs != NULL);
  ASSERT_EQ_INT(OBJ_STRING, vm.objs->type);
  ASSERT_EQ_INT(true, vm.objs->next == NULL);
}

MU_TEST(test_concatenate) {
  ObjString *a = copyString("foo", 3);
  ObjString *b = copyString("bar", 3);

  ObjString *result = concatenate(a, b);

  ASSERT_EQ_INT(true, result != NULL);
  ASSERT_STREQ("foobar", result->chars);
  ASSERT_EQ_INT(6, result->len);
}

MU_TEST_SUITE(object_tests) {
  MU_SUITE_CONFIGURE(&setup, &teardown);

  MU_RUN_TEST(test_copyString);
  MU_RUN_TEST(test_concatenate);
}
