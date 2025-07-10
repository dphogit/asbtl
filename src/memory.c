#include "memory.h"
#include "chunk.h"
#include "compiler.h"
#include "object.h"
#include "vm.h"

#include "debug.h"

#ifdef DEBUG_MODE
#include <stdio.h>
#endif

#include <stdlib.h>

#define GC_HEAP_GROW_FACTOR 2

void *reallocate(void *ptr, size_t newSize, size_t oldSize) {
  vm.bytesAllocated += newSize - oldSize;

  if (newSize > oldSize) {
#ifdef DEBUG_STRESS_GC
    // Trigger the GC everytime we allocate memory (when debugging)
    collectGarbage();
#endif

    if (vm.bytesAllocated > vm.nextGC) {
      collectGarbage();
    }
  }

  if (newSize == 0) {
    free(ptr);
    return NULL;
  }

  void *result = realloc(ptr, newSize);

  if (result == NULL)
    exit(EXIT_FAILURE);

  return result;
}

void markObj(Obj *obj) {
  if (obj == NULL || obj->isMarked)
    return;

#ifdef DEBUG_LOG_GC
  printf("%p mark ", (void *)obj);
  printObj(OBJ_VAL(obj));
  printf("\n");
#endif

  obj->isMarked = true;

  // When an object turns gray, add to the worklist (seen but not processed yet)
  if (vm.grayCount >= vm.grayCapacity) {
    vm.grayCapacity = GROW_CAPACITY(vm.grayCapacity);
    vm.grayStack = (Obj **)realloc(vm.grayStack, sizeof(Obj *) * vm.grayCount);

    if (vm.grayStack == NULL)
      exit(EXIT_FAILURE);
  }

  vm.grayStack[vm.grayCount] = obj;
  vm.grayCount++;
}

void markValue(Value slot) {
  if (IS_OBJ(slot))
    markObj(AS_OBJ(slot));
}

static void markList(ValueList *list) {
  for (unsigned int i = 0; i < list->count; i++) {
    markValue(list->values[i]);
  }
}

// Traverse the single object's references (marks )
static void blackenObj(Obj *obj) {
#ifdef DEBUG_LOG_GC
  printf("%p blacken", (void *)obj);
  printValue(OBJ_VAL(obj));
  printf("\n");
#endif

  switch (obj->type) {
    case OBJ_NATIVE:
    case OBJ_STRING:  break;
    case OBJ_UPVALUE: markValue(((ObjUpvalue *)obj)->closed); break;
    case OBJ_FUNC:    {
      ObjFunc *func = (ObjFunc *)obj;
      markObj((Obj *)func->name);
      markList(&func->chunk.constants);
      break;
    }
    case OBJ_CLOSURE: {
      ObjClosure *closure = (ObjClosure *)obj;
      markObj((Obj *)closure->func);

      for (int i = 0; i < closure->upvalueCount; i++) {
        markObj((Obj *)closure->upvalues[i]);
      }

      break;
    }
  }
}

static void freeObj(Obj *obj) {
#ifdef DEBUG_STRESS_GC
  printf("%p free type %d\n", (void *)obj, obj->type);
#endif

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

static void markRoots() {
  for (Value *slot = vm.stack; slot < vm.stackTop; slot++) {
    markValue(*slot);
  }

  for (int i = 0; i < vm.frameCount; i++) {
    markObj((Obj *)vm.frames[i].closure);
  }

  ObjUpvalue *upvalue = vm.openUpvalues;
  while (upvalue != NULL) {
    markObj((Obj *)upvalue);
    upvalue = upvalue->next;
  }

  markHashTable(&vm.globals);
  markCompilerRoots();
}

static void traceReferences() {
  while (vm.grayCount > 0) {
    vm.grayCount--;

    Obj *obj = vm.grayStack[vm.grayCount];
    blackenObj(obj);
  }
}

static void sweep() {
  Obj *prev = NULL, *cur = NULL;

  // Walk the linked list of the VM's tracked objects on the heap
  while (cur != NULL) {

    // If an object is marked (black), pass it.
    if (cur->isMarked) {
      cur->isMarked = false; // Turn white for next GC cycle
      prev          = cur;
      cur           = cur->next;
      return;
    }

    // If it is unmarked (white - not grey, as worklist is now empty), unlink
    // it from the list and free it.

    Obj *unreached = cur;
    cur            = cur->next;

    if (prev != NULL) {
      prev->next = cur;
    } else {
      vm.objs = cur;
    }

    freeObj(unreached);
  }
}

// Mark and sweep
void collectGarbage() {
#ifdef DEBUG_LOG_GC
  printf("-- GC Begin\n");
  size_t before = vm.bytesAllocated;
#endif

  markRoots();
  traceReferences();
  hashTableRemoveWhite(&vm.strings);
  sweep();

  vm.nextGC = vm.bytesAllocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_LOG_GC
  printf("-- GC End\n");
  printf("   collected %zu bytes (from %zu to %zu) next at %zu\n",
         before - vm.bytesAllocated, before, vm.bytesAllocated, vm.nextGC);
#endif
}

void freeObjs() {
  Obj *cur = vm.objs;

  while (cur != NULL) {
    Obj *next = cur->next;
    freeObj(cur);
    cur = next;
  }

  vm.objs = NULL;
  free(vm.grayStack);
}
