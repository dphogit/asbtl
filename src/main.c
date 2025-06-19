#include "scanner.h"
#include "token.h"

#include <stdbool.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  char *source = "1 + 2 - 3"; // TODO: Read from argv

  Scanner scanner;
  initScanner(&scanner, source);

  while (true) {
    Token tok = scanNext(&scanner);

    if (tok.type == TOK_ERR) {
      printf("TOK_ERR\n");
      break;
    }

    printf("%s '%.*s'\n", tokTypeStr(tok.type), tok.len, tok.start);

    if (tok.type == TOK_EOF)
      break;
  }

  return 0;
}
