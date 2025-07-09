#include "memory.h"
#include "chunk.h"
#include "object.h"
#include "vm.h"

#include <stdlib.h>

void *reallocate(void *ptr, size_t newSize, size_t oldSize) {
  if (newSize == 0) {
    free(ptr);
    return NULL;
  }

  void *result = realloc(ptr, newSize);

  if (result == NULL)
    exit(EXIT_FAILURE);

  return result;
}

static void freeObj(Obj *obj) {
  switch (obj->type) {
    case OBJ_STRING: {
      ObjString *str = (ObjString *)obj;
      FREE_ARRAY(char, (char *)str->chars, str->len + 1); // + 1 for null byte
      FREE(ObjString, str);
      break;
    }
    case OBJ_FUNC: {
      ObjFunc *func = (ObjFunc *)obj;
      freeChunk(&func->chunk);
      FREE(ObjFunc, func);
      break;
    }
    case OBJ_NATIVE: {
      ObjNative *native = (ObjNative *)obj;
      FREE(ObjNative, native);
      break;
    }
    case OBJ_CLOSURE: {
      ObjClosure *closure = (ObjClosure *)obj;
      FREE_ARRAY(ObjUpvalue *, closure->upvalues, closure->upvalueCount);
      FREE(ObjClosure, closure);
      break;
    }
    case OBJ_UPVALUE: {
      ObjUpvalue *upvalue = (ObjUpvalue *)obj;
      FREE(ObjUpvalue, upvalue);
      break;
    }
  }
}

void freeObjs() {
  Obj *cur = vm.objs;

  while (cur != NULL) {
    Obj *next = cur->next;
    freeObj(cur);
    cur = next;
  }

  vm.objs = NULL;
}
