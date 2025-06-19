#include "value.h"
#include "memory.h"

#include <stdlib.h>

void initValueList(ValueList *list) {
  list->capacity = 0;
  list->count    = 0;
  list->values   = NULL;
}

void appendValueList(ValueList *list, Value value) {
  if (list->count >= list->capacity) {
    int oldCap     = list->capacity;
    list->capacity = GROW_CAPACITY(oldCap);
    list->values   = GROW_ARRAY(Value, list->values, oldCap, list->capacity);
  }

  list->values[list->count] = value;
  list->count++;
}

void freeValueList(ValueList *list) {
  FREE_ARRAY(Value, list->values, list->capacity);
  initValueList(list);
}

bool valuesEq(Value a, Value b) {
  return a == b;
}
