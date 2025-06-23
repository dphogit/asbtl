#include "compiler.h"

#include "chunk.h"
#include "scanner.h"
#include "token.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct parser {
  Token cur;
  Token prev;
  Scanner *scanner;
  Chunk *chunk; // Chunk the parser will emit to
  bool hadError;
  bool panicMode;
} Parser;

Parser parser;

static void expression();

static Chunk *currentChunk() {
  return parser.chunk;
}

static void errorAt(Token *token, const char *message) {
  // Suppress cascaded errors which are recovered by panic mode recovery.
  if (parser.panicMode)
    return;

  parser.panicMode = true;

  fprintf(stderr, "[line %d, col %d] error ", token->line, token->col);

  switch (token->type) {
    case TOK_EOF: fprintf(stderr, "at end"); break;
    case TOK_ERR:
      // Do nothing.
      break;
    default: fprintf(stderr, "at '%.*s'", token->len, token->start);
  }

  fprintf(stderr, " %s\n", message);

  parser.hadError = true;
}

static void errorPrev(const char *message) {
  errorAt(&parser.prev, message);
}

static void errorCur(const char *message) {
  errorAt(&parser.cur, message);
}

static void advance() {
  parser.prev = parser.cur;

  while (true) {
    parser.cur = scanNext(parser.scanner);

    if (parser.cur.type == TOK_ERR) {
      errorPrev(parser.cur.start);
      continue;
    }

    return;
  }
}

static void consume(TokType type, char *message) {
  if (parser.cur.type != type) {
    errorCur(message);
    return;
  }

  advance();
}

// Returns whether the parser's current token matches the given token type
static bool check(TokType type) {
  return parser.cur.type == type;
}

// Advances the parser if its current token type matches the given token type.
static bool match(TokType type) {
  if (!check(type))
    return false;

  advance();
  return true;
}

static void emitByte(uint8_t byte) {
  appendChunk(currentChunk(), byte, parser.prev.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2) {
  emitByte(byte1);
  emitByte(byte2);
}

static void emitConstant(Value constant) {
  unsigned int constantIndex = appendConstant(currentChunk(), constant);

  // A byte can only refer to 256 different constant indexes.
  if (constantIndex > UINT8_MAX) {
    parser.hadError = true;
    return;
  }

  emitBytes(OP_CONSTANT, constantIndex);
}

static void emitReturn() {
  emitByte(OP_RETURN);
}

static void number() {
  double value = strtod(parser.prev.start, NULL);
  emitConstant(NUM_VAL(value));
}

static void grouping() {
  expression();
  consume(TOK_RIGHT_PAREN, "expect closing ')' after expression");
}

static void primary() {
  if (match(TOK_NUMBER)) {
    number();
    return;
  }

  if (match(TOK_LEFT_PAREN)) {
    grouping();
    return;
  }

  if (match(TOK_FALSE)) {
    emitByte(OP_FALSE);
    return;
  }

  if (match(TOK_TRUE)) {
    emitByte(OP_TRUE);
    return;
  }

  errorCur("expect expression");
}

static void factor() {
  primary();

  while (match(TOK_STAR) || match(TOK_SLASH)) {
    TokType type = parser.prev.type;

    primary();

    switch (type) {
      case TOK_STAR:  emitByte(OP_MULTIPLY); break;
      case TOK_SLASH: emitByte(OP_DIVIDE); break;
      default:        break;
    }
  }
}

static void expression() {
  factor();

  while (match(TOK_PLUS) || match(TOK_MINUS)) {
    TokType type = parser.prev.type;

    factor();

    switch (type) {
      case TOK_PLUS:  emitByte(OP_ADD); break;
      case TOK_MINUS: emitByte(OP_SUBTRACT); break;
      default:        break;
    }
  }
}

bool compile(const char *source, Chunk *chunk) {
  Scanner scanner;
  initScanner(&scanner, source);

  parser.scanner   = &scanner;
  parser.chunk     = chunk;
  parser.hadError  = false;
  parser.panicMode = false;

  advance();
  expression();
  consume(TOK_EOF, "expected end of expression");
  emitReturn();

  return !parser.hadError;
}
