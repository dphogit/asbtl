#ifndef ASBTL_COMPILER_H
#define ASBTL_COMPILER_H

#include "chunk.h"

// Compile and write to the chunk. Returns true if compilation succeeded.
bool compile(const char *source, Chunk *chunk);

#endif
