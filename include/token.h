#ifndef ASBTL_TOKEN_H
#define ASBTL_TOKEN_H

typedef enum tok_type {
  TOK_PLUS,
  TOK_MINUS,
  TOK_STAR,
  TOK_SLASH,
  TOK_LEFT_PAREN,
  TOK_RIGHT_PAREN,
  TOK_LEFT_BRACE,
  TOK_RIGHT_BRACE,
  TOK_STRING,
  TOK_BANG,
  TOK_BANG_EQ,
  TOK_LESS,
  TOK_LESS_EQ,
  TOK_EQ,
  TOK_EQ_EQ,
  TOK_GREATER,
  TOK_GREATER_EQ,
  TOK_OR,
  TOK_AND,

  TOK_NUMBER,
  TOK_IDENTIFIER,
  TOK_TRUE,
  TOK_FALSE,
  TOK_NIL,
  TOK_VAR,
  TOK_IF,
  TOK_ELSE,
  TOK_WHILE,

  TOK_SEMICOLON,
  TOK_PRINT,
  TOK_ERR,
  TOK_EOF,
} TokType;

char *tokTypeStr(TokType type);

// The token's lexeme points to the starting char within the source program, so
// it needs to have an associated `len` member. This starting char also has
// associated `line` and `col` members for error reporting.
typedef struct token {
  const char *start;
  unsigned int len;
  TokType type;
  unsigned int line;
  unsigned int col;
} Token;

#endif
