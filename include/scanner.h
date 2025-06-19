#ifndef ASBTL_SCANNER_H
#define ASBTL_SCANNER_H

#include "token.h"

typedef struct scanner {
  const char *start; // Start of the lexeme being scanned
  const char *cur;   // Current character in source
  unsigned int line; // Current line scanner is in source
  unsigned int col;  // Current col scanner is in the line/source
} Scanner;

void initScanner(Scanner *scanner, const char *source);

Token scanNext(Scanner *scanner);

#endif
