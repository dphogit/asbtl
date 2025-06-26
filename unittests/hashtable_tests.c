#include "hashtable.h"
#include "minunit.h"

#include <stdbool.h>

HashTable table;

void ht_test_setup(void) {
  initHashTable(&table);
}

void ht_test_teardown(void) {
  freeHashTable(&table);
}

MU_TEST(test_initHashTable) {
  HashTable ht;
  initHashTable(&ht);

  ASSERT_EQ_INT(0, ht.capacity);
  ASSERT_EQ_INT(0, ht.count);
  ASSERT_EQ_INT(true, ht.entries == NULL);

  freeHashTable(&ht);
}

MU_TEST(test_freeHashTable) {
  HashTable ht;
  initHashTable(&ht);

  ObjString key = makeObjString("key", 3);
  hashTableSet(&ht, &key, NUM_VAL(42));

  freeHashTable(&ht);

  ASSERT_EQ_INT(0, ht.capacity);
  ASSERT_EQ_INT(0, ht.count);
  ASSERT_EQ_INT(true, ht.entries == NULL);
}

MU_TEST(test_hashTableSet_newEntry) {
  ObjString key = makeObjString("key", 3);

  bool isNew = hashTableSet(&table, &key, NUM_VAL(42));

  ASSERT_EQ_INT(true, isNew);
  ASSERT_EQ_INT(1, table.count);
  ASSERT_EQ_INT(true, table.capacity > 1);

  Value value;
  ASSERT_EQ_INT(true, hashTableGet(&table, &key, &value));
  ASSERT_EQ_INT(true, IS_NUM(value));
  ASSERT_EQ_INT(42, AS_NUM(value));
}

MU_TEST(test_hashTableSet_overwriteKey) {
  ObjString key = makeObjString("key", 3);
  hashTableSet(&table, &key, NUM_VAL(42));

  bool isNew = hashTableSet(&table, &key, NUM_VAL(69));

  ASSERT_EQ_INT(false, isNew);
  ASSERT_EQ_INT(1, table.count);
  ASSERT_EQ_INT(true, table.capacity > 1);

  Value value;
  ASSERT_EQ_INT(true, hashTableGet(&table, &key, &value));
  ASSERT_EQ_INT(true, IS_NUM(value));
  ASSERT_EQ_INT(69, AS_NUM(value));
}

MU_TEST(test_hashTableGet) {
  ObjString key = makeObjString("key", 3);
  hashTableSet(&table, &key, NUM_VAL(42));
  Value value;

  bool exists = hashTableGet(&table, &key, &value);

  ASSERT_EQ_INT(true, exists);
  ASSERT_EQ_INT(true, IS_NUM(value));
  ASSERT_EQ_INT(42, AS_NUM(value));
}

MU_TEST(test_hashTableGet_notExist) {
  ObjString key = makeObjString("key", 3);
  Value value;

  bool exists = hashTableGet(&table, &key, &value);

  ASSERT_EQ_INT(false, exists);
}

MU_TEST(test_hashTableRemove) {
  ObjString key = makeObjString("key", 3);
  hashTableSet(&table, &key, NUM_VAL(42));

  bool success = hashTableRemove(&table, &key);

  ASSERT_EQ_INT(true, success);

  Value value;
  ASSERT_EQ_INT(false, hashTableGet(&table, &key, &value));
}

MU_TEST(test_hashTableRemove_notExist) {
  ObjString key = makeObjString("key", 3);

  bool success = hashTableRemove(&table, &key);

  ASSERT_EQ_INT(false, success);
}

MU_TEST_SUITE(hashtable_tests) {
  MU_SUITE_CONFIGURE(&ht_test_setup, &ht_test_teardown);

  MU_RUN_TEST(test_initHashTable);
  MU_RUN_TEST(test_freeHashTable);
  MU_RUN_TEST(test_hashTableSet_newEntry);
  MU_RUN_TEST(test_hashTableSet_overwriteKey);
  MU_RUN_TEST(test_hashTableGet);
  MU_RUN_TEST(test_hashTableGet_notExist);
  MU_RUN_TEST(test_hashTableRemove);
  MU_RUN_TEST(test_hashTableRemove_notExist);
}
