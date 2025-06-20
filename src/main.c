#include "vm.h"

#include <stdbool.h>

int main(int argc, char *argv[]) {
  // TODO: REPL, Run File
  char *source = "1 + 2 - 3";

  initVM();

  InterpretResult result = interpret(source);

  freeVM();
  return result;
}
