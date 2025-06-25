#include "object.h"
#include "memory.h"
#include "vm.h"

#include <stdio.h>
#include <string.h>

#define ALLOCATE_OBJ(type, objType) (type *)allocateObj(sizeof(type), objType)

static Obj *allocateObj(size_t size, ObjType type) {
  Obj *obj  = reallocate(NULL, size, 0);
  obj->type = type;

  // Prepend allocated object to VM's intrusive linked list
  obj->next = vm.objs;
  vm.objs   = obj;

  return obj;
}

static ObjString *allocateString(char *chars, int n) {
  ObjString *str = ALLOCATE_OBJ(ObjString, OBJ_STRING);
  str->chars     = chars;
  str->len       = n;
  return str;
}

ObjString *copyString(const char *chars, int n) {
  char *heapChars = ALLOCATE(char, n + 1);
  strncpy(heapChars, chars, n);
  heapChars[n] = '\0';
  return allocateString(heapChars, n);
}

ObjString *takeString(char *chars, int n) {
  return allocateString(chars, n);
}

ObjString *concatenate(ObjString *a, ObjString *b) {
  int n       = a->len + b->len;
  char *chars = ALLOCATE(char, n + 1);

  strncpy(chars, a->chars, a->len);
  strncpy(chars + a->len, b->chars, b->len);
  chars[n] = '\0';

  return takeString(chars, n);
}

void printObj(Value value) {
  switch (OBJ_TYPE(value)) {
    case OBJ_STRING: printf("%s", AS_CSTRING(value)); break;
  }
}
