#ifndef ASBTL_VALUE_H
#define ASBTL_VALUE_H

#include <stdbool.h>

typedef double Value;

typedef struct value_list {
  unsigned int capacity;
  unsigned int count;
  Value *values;
} ValueList;

void initValueList(ValueList *list);
void appendValueList(ValueList *list, Value value);
void freeValueList(ValueList *list);

void printValue(Value value);
bool valuesEq(Value a, Value b);

#endif
