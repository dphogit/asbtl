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
  initScanner(&scanner, "+ - * / ( ) { } ! ; = < <= == != >= > || && 123 false "
                        "true nil \"string\" var myVar if else");

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
  ASSERT_TOK(tok, "{", 1, TOK_LEFT_BRACE);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, "}", 1, TOK_RIGHT_BRACE);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, "!", 1, TOK_BANG);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, ";", 1, TOK_SEMICOLON);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, "=", 1, TOK_EQ);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, "<", 1, TOK_LESS);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, "<=", 2, TOK_LESS_EQ);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, "==", 2, TOK_EQ_EQ);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, "!=", 2, TOK_BANG_EQ);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, ">=", 2, TOK_GREATER_EQ);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, ">", 1, TOK_GREATER);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, "||", 2, TOK_OR);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, "&&", 2, TOK_AND);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, "123", 3, TOK_NUMBER);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, "false", 5, TOK_FALSE);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, "true", 4, TOK_TRUE);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, "nil", 3, TOK_NIL);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, "\"string\"", 8, TOK_STRING);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, "var", 3, TOK_VAR);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, "myVar", 5, TOK_IDENTIFIER);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, "if", 2, TOK_IF);

  tok = scanNext(&scanner);
  ASSERT_TOK(tok, "else", 4, TOK_ELSE);

  tok = scanNext(&scanner);
  ASSERT_EQ_INT(TOK_EOF, tok.type);

#undef ASSERT_TOK
}

MU_TEST_SUITE(scanner_tests) {
  MU_RUN_TEST(test_initScanner);
  MU_RUN_TEST(test_scanNext);
}
