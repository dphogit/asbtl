#include "minunit.h"
#include "object.h"
#include "test_runners.h"
#include "vm.h"

#include <stdbool.h>

void object_test_setup() {
  initVM();
}

void object_test_teardown() {
  freeVM();
}

MU_TEST(test_makeObjString) {
  ObjString s = makeObjString("foo", 3);

  ASSERT_EQ_INT(3, s.len);
  ASSERT_STREQ("foo", s.chars);
  ASSERT_EQ_INT(OBJ_STRING, s.obj.type);
  ASSERT_EQ_INT(true, s.obj.next == NULL);
}

MU_TEST(test_hashString_consistency) {
  const char *key1 = "foo";
  const char *key2 = "foo";
  int n            = 3;

  uint32_t h1 = hashString(key1, n);
  uint32_t h2 = hashString(key2, n);

  ASSERT_EQ_INT(h1, h2);
}

MU_TEST(test_hashString_difference) {
  const char *key1 = "foo";
  const char *key2 = "bar";
  int n            = 3;

  uint32_t h1 = hashString(key1, n);
  uint32_t h2 = hashString(key2, n);

  ASSERT_EQ_INT(false, h1 == h2);
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
  MU_SUITE_CONFIGURE(&object_test_setup, &object_test_teardown);

  MU_RUN_TEST(test_makeObjString);
  MU_RUN_TEST(test_hashString_consistency);
  MU_RUN_TEST(test_hashString_difference);
  MU_RUN_TEST(test_copyString);
  MU_RUN_TEST(test_concatenate);
}
