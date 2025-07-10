#ifndef ASBTL_DEBUG_H
#define ASBTL_DEBUG_H

#ifdef DEBUG_MODE
#define DEBUG_STRESS_GC
#define DEBUG_LOG_GC
#endif

#include "chunk.h"

void disassembleChunk(Chunk *chunk, const char *name);

#endif
