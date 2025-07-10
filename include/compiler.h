#ifndef ASBTL_COMPILER_H
#define ASBTL_COMPILER_H

#include "object.h"

#define UPVALUE_CAPTURES_UPVALUE (0)
#define UPVALUE_CAPTURES_LOCAL   (1)

// Returns the object function contianing the chunk with bytecode if compilation
// succeeded, otherwise a failure will return NULL.
ObjFunc *compile(const char *source);

void markCompilerRoots();

#endif
