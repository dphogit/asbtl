#ifndef ASBTL_OBJECT_H
#define ASBTL_OBJECT_H

#include "value.h"

#define OBJ_TYPE(value)   AS_OBJ(value)->type

#define IS_STRING(value)  isObjType(value, OBJ_STRING)
#define AS_STRING(value)  ((ObjString *)(value).as.obj)
#define AS_CSTRING(value) (((ObjString *)(value.as.obj))->chars)

typedef enum obj_type {
  OBJ_STRING
} ObjType;

struct obj {
  ObjType type;
  struct obj *next; // For GC
};

typedef struct obj_string {
  Obj obj;
  char *chars;
  int len;
} ObjString;

static inline bool isObjType(Value value, ObjType type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

// Conservatively makes a new heap-allocation of the passed chars
ObjString *copyString(const char *chars, int n);

// Claims ownership of the heap-allocated passed chars
ObjString *takeString(char *chars, int n);

ObjString *concatenate(ObjString *a, ObjString *b);

void printObj(Value value);

#endif
