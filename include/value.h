#ifndef ASBTL_VALUE_H
#define ASBTL_VALUE_H

#include <stdbool.h>

typedef enum value_type {
  VAL_NIL,
  VAL_BOOL,
  VAL_NUM,
  VAL_OBJ
} ValueType;

typedef struct obj Obj;

typedef struct value {
  ValueType type;
  union as {
    bool boolean;
    double number;
    Obj *obj;
  } as;
} Value;

// `Value` type checkers
#define IS_NIL(value)  ((value).type == VAL_NIL)
#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NUM(value)  ((value).type == VAL_NUM)
#define IS_OBJ(value)  ((value).type == VAL_OBJ)

// Convert from primitive C type to `Value` type
#define NIL_VAL         ((Value){VAL_NIL, {.number = 0}})
#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define NUM_VAL(value)  ((Value){VAL_NUM, {.number = value}})
#define OBJ_VAL(value)  ((Value){VAL_OBJ, {.obj = (Obj *)value}})

// Convert from `Value` type from primitive C type
#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUM(value)  ((value).as.number)
#define AS_OBJ(value)  ((value).as.obj)

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
