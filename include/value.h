#ifndef ASBTL_VALUE_H
#define ASBTL_VALUE_H

#include <stdbool.h>

typedef enum value_type { VAL_BOOL, VAL_NUM } ValueType;

typedef struct value {
  ValueType type;
  union as {
    bool boolean;
    double number;
  } as;
} Value;

// `Value` type checkers
#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NUM(value)  ((value).type == VAL_NUM)

// Convert from primitive C type to `Value` type
#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define NUM_VAL(value)  ((Value){VAL_NUM, {.number = value}})

// Convert from `Value` type from primitive C type
#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUM(value)  ((value).as.number)

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
