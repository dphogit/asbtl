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
