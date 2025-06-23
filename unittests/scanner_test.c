#include "scanner.h"
#include "test_runners.h"
#include "token.h"

#include "minunit.h"

MU_TEST(test_initScanner) {
  Scanner scanner;

  initScanner(&scanner, "1 + 2");

  ASSERT_EQ_INT('1', *scanner.start);
  ASSERT_EQ_INT('1', *scanner.cur);
  ASSERT_EQ_INT(1, scanner.line);
  ASSERT_EQ_INT(1, scanner.col);
}

MU_TEST(test_scanNext_number) {
  Scanner scanner;
  initScanner(&scanner, "123");

  Token tok = scanNext(&scanner);

  ASSERT_EQ_INT('1', *tok.start);
  ASSERT_EQ_INT(3, tok.len);
  ASSERT_EQ_INT(TOK_NUMBER, tok.type);
}

MU_TEST(test_scanNext_operators) {
#define ASSERT_OP(tok, expStart, expLen, expType) \
  ASSERT_EQ_INT(expStart, *tok.start);            \
  ASSERT_EQ_INT(expLen, tok.len);                 \
  ASSERT_EQ_INT(expType, tok.type);

  Scanner scanner;
  initScanner(&scanner, "+ - * /");

  Token tok = scanNext(&scanner);
  ASSERT_OP(tok, '+', 1, TOK_PLUS);

  tok = scanNext(&scanner);
  ASSERT_OP(tok, '-', 1, TOK_MINUS);

  tok = scanNext(&scanner);
  ASSERT_OP(tok, '*', 1, TOK_STAR);

  tok = scanNext(&scanner);
  ASSERT_OP(tok, '/', 1, TOK_SLASH);
}

MU_TEST_SUITE(scanner_tests) {
  MU_RUN_TEST(test_initScanner);
  MU_RUN_TEST(test_scanNext_number);
  MU_RUN_TEST(test_scanNext_operators);
}
