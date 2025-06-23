#include "value.h"
#include "memory.h"

#include <stdio.h>
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

void printValue(Value value) {
  switch (value.type) {
    case VAL_NIL:  printf("nil"); break;
    case VAL_BOOL: printf("%s", AS_BOOL(value) ? "true" : "false"); break;
    case VAL_NUM:  printf("%g", AS_NUM(value)); break;
  }
}

bool valuesEq(Value a, Value b) {
  if (a.type != b.type)
    return false;

  switch (a.type) {
    case VAL_NIL:  return false;
    case VAL_BOOL: return AS_BOOL(a) == AS_BOOL(b);
    case VAL_NUM:  return AS_NUM(a) == AS_NUM(b);
  }

  return false;
}
