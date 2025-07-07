#ifndef ASBTL_OBJECT_H
#define ASBTL_OBJECT_H

#include "chunk.h"
#include "value.h"

#include <stdint.h>

#define OBJ_TYPE(value)   AS_OBJ(value)->type

#define IS_STRING(value)  isObjType(value, OBJ_STRING)
#define IS_FUNC(value)    isObjType(value, OBJ_FUNC)
#define IS_NATIVE(value)  isObjType(value, OBJ_NATIVE)

#define AS_STRING(value)  ((ObjString *)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString *)AS_OBJ(value))->chars)
#define AS_FUNC(value)    ((ObjFunc *)AS_OBJ(value))
#define AS_NATIVE(value)  (((ObjNative *)AS_OBJ(value))->func)

typedef enum obj_type {
  OBJ_STRING,
  OBJ_FUNC,
  OBJ_NATIVE,
} ObjType;

struct obj {
  ObjType type;
  struct obj *next; // For GC
};

typedef struct obj_string {
  Obj obj;
  const char *chars;
  int len;
  uint32_t hash;
} ObjString;

typedef struct obj_func {
  Obj obj;
  int arity;   // Number of parameters the function expects
  Chunk chunk; // Each function has its own chunk
  ObjString *name;
} ObjFunc;

// A native function takes an argument count and pointer to the first argument
// on the value stack. Once done, returns the result value.
typedef Value (*NativeFn)(int argCount, Value *args);

typedef struct obj_native {
  Obj obj;
  NativeFn func;
} ObjNative;

static inline bool isObjType(Value value, ObjType type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

uint32_t hashString(const char *key, int n);

ObjFunc *newFunc();
ObjNative *newNative(NativeFn func);

ObjString makeObjString(const char *chars, int n);

// Conservatively makes a new heap-allocation of the passed chars
ObjString *copyString(const char *chars, int n);

// Claims ownership of the heap-allocated passed chars
ObjString *takeString(char *chars, int n);

ObjString *concatenate(ObjString *a, ObjString *b);

void printObj(Value value);

#endif
