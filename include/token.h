#ifndef ASBTL_TOKEN_H
#define ASBTL_TOKEN_H

typedef enum tok_type {
  TOK_PLUS,
  TOK_MINUS,
  TOK_STAR,
  TOK_SLASH,
  TOK_LEFT_PAREN,
  TOK_RIGHT_PAREN,

  TOK_NUMBER,

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
