#include "vm.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>

static void repl() {
  initVM();

  char *line  = NULL;
  size_t size = 0;
  ssize_t nread;

  printf("Welcome to ASBTL - A Stack Based Toy Language\n");

  while (true) {
    printf("> ");

    if ((nread = getline(&line, &size, stdin)) == -1) {
      break;
    }

    if (line[nread - 1] == '\n') {
      line[nread - 1] = '\0';
    }

    interpret(line);
  }

  free(line);
  freeVM();
}

char *readFile(const char *path) {
  FILE *file = fopen(path, "r");
  if (file == NULL) {
    fprintf(stderr, "could not open file '%s'\n", path);
    exit(EX_IOERR);
  }

  if (fseek(file, 0, SEEK_END) == -1) {
    perror("fseek");
    exit(EX_IOERR);
  }

  long bytes = ftell(file);
  if (bytes == -1) {
    perror("ftell");
    exit(EX_IOERR);
  }

  if (fseek(file, 0, SEEK_SET) == -1) {
    perror("fseek");
    exit(EX_IOERR);
  }

  char *source = malloc(bytes + 1); // Extra byte for '\0'
  if (source == NULL) {
    perror("malloc");
    exit(EX_IOERR);
  }

  long bytesRead = fread(source, sizeof(char), bytes, file);
  if (bytesRead < bytes) {
    fprintf(stderr, "failed to read file '%s'\n", path);
    exit(EX_IOERR);
  }

  fclose(file);

  source[bytes] = '\0';
  return source;
}

void runFile(const char *path) {
  initVM();

  char *source = readFile(path);

  InterpretResult result = interpret(source);

  free(source);
  freeVM();

  if (result != INTERPRET_OK)
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    repl();
  } else if (argc == 2) {
    runFile(argv[1]);
  } else {
    fprintf(stderr, "usage: %s [script]\n", argv[0]);
    exit(EX_USAGE);
  }
}
