#include "test_runners.h"

#include "minunit.h"
#include "value.h"

#define ASSERT_VALUELIST_INIT(list)           \
  do {                                        \
    ASSERT_EQ_INT(0, list.capacity);          \
    ASSERT_EQ_INT(0, list.count);             \
    ASSERT_EQ_INT(true, list.values == NULL); \
  } while (0);

MU_TEST(test_initValueList) {
  ValueList list;

  initValueList(&list);

  ASSERT_VALUELIST_INIT(list);

  freeValueList(&list);
}

MU_TEST(test_appendValueList) {
  ValueList list;
  initValueList(&list);

  appendValueList(&list, 0.5);

  ASSERT_EQ_INT(1, list.count);
  ASSERT_GE(list.capacity, 1, ""); // Assert capacity grew from reallocation
  ASSERT_EQ_INT(true, valuesEq(list.values[0], 0.5));

  freeValueList(&list);
}

MU_TEST(test_freeValueList) {
  ValueList list;
  initValueList(&list);

  freeValueList(&list);

  ASSERT_VALUELIST_INIT(list);
}

MU_TEST(test_valuesEq_double) {
  Value a = 0.5, b = 0.5;
  ASSERT_EQ_INT(true, valuesEq(a, b));
}

MU_TEST(test_valuesEq_doubleNotEq) {
  Value a = 0.4, b = 0.5;
  ASSERT_EQ_INT(false, valuesEq(a, b));
}

MU_TEST_SUITE(value_tests) {
  MU_RUN_TEST(test_initValueList);
  MU_RUN_TEST(test_appendValueList);
  MU_RUN_TEST(test_freeValueList);

  MU_RUN_TEST(test_valuesEq_double);
  MU_RUN_TEST(test_valuesEq_doubleNotEq);
}
