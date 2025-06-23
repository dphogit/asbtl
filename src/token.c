#include "token.h"

char *tokTypeStr(TokType type) {
  switch (type) {
    case TOK_PLUS:        return "TOK_PLUS";
    case TOK_MINUS:       return "TOK_MINUS";
    case TOK_STAR:        return "TOK_STAR";
    case TOK_SLASH:       return "TOK_SLASH";
    case TOK_LEFT_PAREN:  return "TOK_LEFT_PAREN";
    case TOK_RIGHT_PAREN: return "TOK_RIGHT_PAREN";
    case TOK_BANG:        return "TOK_BANG";
    case TOK_NUMBER:      return "TOK_NUMBER";
    case TOK_IDENTIFIER:  return "TOK_IDENTIFIER";
    case TOK_TRUE:        return "TOK_TRUE";
    case TOK_FALSE:       return "TOK_FALSE";
    case TOK_NIL:         return "TOK_NIL";
    case TOK_ERR:         return "TOK_ERR";
    case TOK_EOF:         return "TOK_EOF";
  }

  return "unknown token type";
}
