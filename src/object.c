#include "object.h"
#include "hashtable.h"
#include "memory.h"
#include "vm.h"

#include <stdio.h>
#include <string.h>

#define FNV_32_OFFSET_BASIS         2166136261u
#define FNV_32_PRIME                16777619

#define ALLOCATE_OBJ(type, objType) (type *)allocateObj(sizeof(type), objType)

static Obj *allocateObj(size_t size, ObjType type) {
  Obj *obj  = reallocate(NULL, size, 0);
  obj->type = type;

  // Prepend allocated object to VM's intrusive linked list
  obj->next = vm.objs;
  vm.objs   = obj;

  return obj;
}

ObjFunc *newFunc() {
  ObjFunc *func = ALLOCATE_OBJ(ObjFunc, OBJ_FUNC);
  func->arity   = 0;
  func->name    = NULL;

  initChunk(&func->chunk);

  return func;
}

ObjNative *newNative(NativeFn func) {
  ObjNative *native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
  native->func      = func;
  return native;
}

static ObjString *allocateObjString(char *chars, int n, uint32_t hash) {
  ObjString *str = ALLOCATE_OBJ(ObjString, OBJ_STRING);
  str->chars     = chars;
  str->len       = n;
  str->hash      = hash;

  hashTableSet(&vm.strings, str, NIL_VAL);

  return str;
}

ObjString makeObjString(const char *chars, int n) {
  Obj obj       = {OBJ_STRING, NULL};
  ObjString str = {obj, chars, n, hashString(chars, n)};
  return str;
}

// FNV-1a hash function:
// https://en.wikipedia.org/wiki/Fowler-Noll-Vo_hash_function
uint32_t hashString(const char *key, int n) {
  uint32_t hash = FNV_32_OFFSET_BASIS;

  for (int i = 0; i < n; i++) {
    hash ^= (uint8_t)key[i];
    hash *= FNV_32_PRIME;
  }

  return hash;
}

ObjString *copyString(const char *chars, int n) {
  uint32_t hash = hashString(chars, n);

  ObjString *interned = tableFindString(&vm.strings, chars, n, hash);
  if (interned != NULL) {
    return interned;
  }

  char *heapChars = ALLOCATE(char, n + 1);
  strncpy(heapChars, chars, n);
  heapChars[n] = '\0';

  return allocateObjString(heapChars, n, hash);
}

ObjString *takeString(char *chars, int n) {
  uint32_t hash = hashString(chars, n);

  ObjString *interned = tableFindString(&vm.strings, chars, n, hash);
  if (interned != NULL) {
    FREE_ARRAY(char, chars, n + 1); // + 1 for null byte
    return interned;
  }

  return allocateObjString(chars, n, hash);
}

ObjString *concatenate(ObjString *a, ObjString *b) {
  int n       = a->len + b->len;
  char *chars = ALLOCATE(char, n + 1);

  strncpy(chars, a->chars, a->len);
  strncpy(chars + a->len, b->chars, b->len);
  chars[n] = '\0';

  return takeString(chars, n);
}

static void printFunc(ObjFunc *func) {
  if (func->name == NULL) {
    printf("<script>");
    return;
  }

  printf("<fn %s>", func->name->chars);
}

void printObj(Value value) {
  switch (OBJ_TYPE(value)) {
    case OBJ_STRING: printf("%s", AS_CSTRING(value)); break;
    case OBJ_FUNC:   printFunc(AS_FUNC(value)); break;
    case OBJ_NATIVE: printf("<native fn>"); break;
  }
}
