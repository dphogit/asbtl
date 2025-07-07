#ifndef ASBTL_COMPILER_H
#define ASBTL_COMPILER_H

#include "object.h"

// Returns the object function contianing the chunk with bytecode if compilation
// succeeded, otherwise a failure will return NULL.
ObjFunc *compile(const char *source);

#endif
