#include "chunk.h"
#include "compiler.h"
#include "debug.h"

#include <stdbool.h>

int main(int argc, char *argv[]) {
  char *source = "1 + 2 - 3"; // TODO: Read from argv
  int exitCode = 1;

  Chunk chunk;
  initChunk(&chunk);

  if (compile(source, &chunk)) {
    disassembleChunk(&chunk, "main");
    exitCode = 0;
  }

  freeChunk(&chunk);
  return exitCode;
}
