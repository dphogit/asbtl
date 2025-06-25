#include "compiler.h"

#include "chunk.h"
#include "object.h"
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

  fprintf(stderr, "[line %d, col %d] error", token->line, token->col);

  switch (token->type) {
    case TOK_EOF: fprintf(stderr, " at end"); break;
    case TOK_ERR: break; // Nothing.
    default:      fprintf(stderr, " at '%.*s'", token->len, token->start);
  }

  fprintf(stderr, ": %s\n", message);

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
      errorCur(parser.cur.start);
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

/*
 * Emits the supplied jump instruction with a 2-byte placeholder operand. The
 * operand should then be backpatched with patchJump(). Returns the offset of
 * the operand's first byte (i.e. high byte).
 */
static int emitJump(OpCode jumpInstruction) {
  emitByte(jumpInstruction);
  emitByte(0xFF); // High byte
  emitByte(0xFF); // Low byte
  return currentChunk()->count - 2;
}

/*
 * Given the offset of the first byte of the jump instruction's operand,
 * backpatch the placeholder value with the correct number of bytes to jump.
 * Calculated by taking the chunks current count - offset - 2.
 */
static void patchJump(int offset) {
  Chunk *chunk    = currentChunk();
  int bytesToJump = chunk->count - offset - 2;

  // 2-byte operand => max value of 65535
  if (bytesToJump > UINT16_MAX) {
    errorPrev("exceeded max of 65535 bytes to jump over");
  }

  // Split and write the 2-byte (16-bit) operand into the respective bytes
  chunk->code[offset]     = (bytesToJump >> 8) & 0xFF; // High byte
  chunk->code[offset + 1] = bytesToJump & 0xFF;        // Low byte
}

static void emitConstant(Value constant) {
  unsigned int constantIndex = appendConstant(currentChunk(), constant);

  if (constantIndex > UINT8_MAX) {
    // A byte can only refer to 256 different constant indexes.
    errorPrev("exceeded max of 256 constants in one chunk");
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

static void string() {
  // Trim surrounding quotes
  const char *chars = parser.prev.start + 1;
  int n             = parser.prev.len - 2;

  ObjString *str = copyString(chars, n);
  emitConstant(OBJ_VAL(str));
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

  if (match(TOK_STRING)) {
    string();
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

  if (match(TOK_NIL)) {
    emitByte(OP_NIL);
    return;
  }

  errorCur("expect expression");
}

static void unary() {
  // allow recursive calls to itself for multiple prefixes e.g. "--5"
  if (match(TOK_BANG)) {
    unary();
    emitByte(OP_NOT);
    return;
  }

  if (match(TOK_MINUS)) {
    unary();
    emitByte(OP_NEGATE);
    return;
  }

  primary();
}

static void factor() {
  unary();

  while (match(TOK_STAR) || match(TOK_SLASH)) {
    TokType type = parser.prev.type;

    unary();

    switch (type) {
      case TOK_STAR:  emitByte(OP_MULTIPLY); break;
      case TOK_SLASH: emitByte(OP_DIVIDE); break;
      default:        break;
    }
  }
}

static void term() {
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

static void comparison() {
  term();

  while (match(TOK_LESS) || match(TOK_LESS_EQ) || match(TOK_GREATER) ||
         match(TOK_GREATER_EQ)) {
    TokType type = parser.prev.type;

    term();

    switch (type) {
      case TOK_LESS:       emitByte(OP_LESS); break;
      case TOK_LESS_EQ:    emitByte(OP_LESS_EQ); break;
      case TOK_GREATER_EQ: emitByte(OP_GREATER_EQ); break;
      case TOK_GREATER:    emitByte(OP_GREATER); break;
      default:             break;
    }
  }
}

static void equality() {
  comparison();

  while (match(TOK_EQ_EQ) || match(TOK_BANG_EQ)) {
    TokType type = parser.prev.type;

    comparison();

    switch (type) {
      case TOK_EQ_EQ:   emitByte(OP_EQ); break;
      case TOK_BANG_EQ: emitByte(OP_NOT_EQ); break;
      default:          break;
    }
  }
}

static void logicalAnd() {
  equality();

  while (match(TOK_AND)) {
    int operandOffset = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP); // if LHS falsy, pop removes it from stack
    equality();
    patchJump(operandOffset);
  }
}

static void logicalOr() {
  logicalAnd();

  while (match(TOK_OR)) {
    int operandOffset = emitJump(OP_JUMP_IF_TRUE);
    emitByte(OP_POP); // if LHS falsy, pop removes it from stack
    logicalAnd();
    patchJump(operandOffset);
  }
}

static void expression() {
  logicalOr();
}

static void printStmt() {
  expression();
  consume(TOK_SEMICOLON, "expect ';' at end");
  emitByte(OP_PRINT);
}

static void exprStmt() {
  expression();
  consume(TOK_SEMICOLON, "expect ';' at end");
  emitByte(OP_POP);
}

static void statement() {
  if (match(TOK_PRINT)) {
    printStmt();
    return;
  }

  exprStmt();
}

static void endCompiler() {
  emitReturn();
}

bool compile(const char *source, Chunk *chunk) {
  Scanner scanner;
  initScanner(&scanner, source);

  parser.scanner   = &scanner;
  parser.chunk     = chunk;
  parser.hadError  = false;
  parser.panicMode = false;

  advance();

  while (!match(TOK_EOF)) {
    statement();
  }

  endCompiler();
  return !parser.hadError;
}
