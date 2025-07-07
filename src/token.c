#include "token.h"

char *tokTypeStr(TokType type) {
  switch (type) {
    case TOK_PLUS:        return "TOK_PLUS";
    case TOK_MINUS:       return "TOK_MINUS";
    case TOK_STAR:        return "TOK_STAR";
    case TOK_SLASH:       return "TOK_SLASH";
    case TOK_LEFT_PAREN:  return "TOK_LEFT_PAREN";
    case TOK_RIGHT_PAREN: return "TOK_RIGHT_PAREN";
    case TOK_LEFT_BRACE:  return "TOK_LEFT_BRACE";
    case TOK_RIGHT_BRACE: return "TOK_RIGHT_BRACE";
    case TOK_COMMA:       return "TOK_COMMA";
    case TOK_COLON:       return "TOK_COLON";
    case TOK_SEMICOLON:   return "TOK_SEMICOLON";
    case TOK_STRING:      return "TOK_STRING";
    case TOK_BANG:        return "TOK_BANG";
    case TOK_BANG_EQ:     return "TOK_BANG_EQ";
    case TOK_EQ:          return "TOK_EQ";
    case TOK_EQ_EQ:       return "TOK_EQ_EQ";
    case TOK_LESS:        return "TOK_LESS";
    case TOK_LESS_EQ:     return "TOK_LESS_EQ";
    case TOK_GREATER:     return "TOK_GREATER";
    case TOK_GREATER_EQ:  return "TOK_GREATER_EQ";
    case TOK_OR:          return "TOK_OR";
    case TOK_AND:         return "TOK_AND";
    case TOK_QUESTION:    return "TOK_QUESTION";
    case TOK_NUMBER:      return "TOK_NUMBER";
    case TOK_IDENTIFIER:  return "TOK_IDENTIFIER";
    case TOK_TRUE:        return "TOK_TRUE";
    case TOK_FALSE:       return "TOK_FALSE";
    case TOK_NIL:         return "TOK_NIL";
    case TOK_VAR:         return "TOK_VAR";
    case TOK_IF:          return "TOK_IF";
    case TOK_ELSE:        return "TOK_ELSE";
    case TOK_WHILE:       return "TOK_WHILE";
    case TOK_FOR:         return "TOK_FOR";
    case TOK_FUNC:        return "TOK_FUNC";
    case TOK_PRINT:       return "TOK_PRINT";
    case TOK_ERR:         return "TOK_ERR";
    case TOK_EOF:         return "TOK_EOF";
  }

  return "unknown token type";
}
