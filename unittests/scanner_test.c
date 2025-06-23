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

MU_TEST(test_scanNext) {
#define ASSERT_TOK(tok, expStart, expLen, expType) \
  ASSERT_EQ_INT(expLen, tok.len);                  \
  for (int i = 0; i < expLen; i++)                 \
    ASSERT_EQ_INT(expStart[i], tok.start[i]);      \
  ASSERT_EQ_INT(expType, tok.type);

  Scanner scanner;
  initScanner(&scanner, "+ - * / ( ) 123");

  Token tok = scanNext(&scanner);
  ASSERT_TOK(tok, "+", 1, TOK_PLUS);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, "-", 1, TOK_MINUS);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, "*", 1, TOK_STAR);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, "/", 1, TOK_SLASH);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, "(", 1, TOK_LEFT_PAREN);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, ")", 1, TOK_RIGHT_PAREN);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, "123", 3, TOK_NUMBER);
#undef ASSERT_TOK
}

MU_TEST_SUITE(scanner_tests) {
  MU_RUN_TEST(test_initScanner);
  MU_RUN_TEST(test_scanNext);
}
