#include "token.h"

char *tokTypeStr(TokType type) {
  switch (type) {
    case TOK_PLUS:   return "TOK_PLUS";
    case TOK_MINUS:  return "TOK_MINUS";
    case TOK_STAR:   return "TOK_STAR";
    case TOK_SLASH:  return "TOK_SLASH";
    case TOK_NUMBER: return "TOK_NUMBER";
    case TOK_ERR:    return "TOK_ERR";
    case TOK_EOF:    return "TOK_EOF";
  }

  return "unknown token type";
}
