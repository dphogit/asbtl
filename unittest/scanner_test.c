#include "scanner.h"
#include "token.h"

#include "minunit.h"

MU_TEST(test_initScanner) {
  Scanner scanner;

  initScanner(&scanner, "1 + 2");

  mu_check(*scanner.start == '1');
  mu_check(*scanner.cur == '1');
  mu_check(scanner.line == 1);
  mu_check(scanner.col == 1);
}

MU_TEST(test_scanNext_number) {
  Scanner scanner;
  initScanner(&scanner, "123");

  Token tok = scanNext(&scanner);

  mu_check(*tok.start == '1');
  mu_check(tok.len == 3);
  mu_check(tok.type == TOK_NUMBER);
}

MU_TEST(test_scanNext_operators) {
  Scanner scanner;
  initScanner(&scanner, "+ -");

  Token tok = scanNext(&scanner);
  mu_check(*tok.start == '+');
  mu_check(tok.len == 1);
  mu_check(tok.type == TOK_PLUS);

  tok = scanNext(&scanner);
  mu_check(*tok.start == '-');
  mu_check(tok.len == 1);
  mu_check(tok.type == TOK_MINUS);
}

MU_TEST_SUITE(scanner_tests) {
  MU_RUN_TEST(test_initScanner);
  MU_RUN_TEST(test_scanNext_number);
  MU_RUN_TEST(test_scanNext_operators);
}

int main(void) {
  MU_RUN_SUITE(scanner_tests);
  MU_REPORT();
  return MU_EXIT_CODE;
}
