#include "compiler.h"

#include "chunk.h"
#include "object.h"
#include "scanner.h"
#include "token.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct parser {
  Token cur;
  Token prev;
  Scanner *scanner;
  Chunk *chunk; // Chunk the parser will emit to
  bool hadError;
  bool panicMode;
} Parser;

// Represents a local variable
typedef struct local {
  Token name;
  int depth;
} Local;

typedef struct compiler {
  Local locals[UINT8_MAX + 1];
  int localCount;
  int scopeDepth; // Number of surrounding blocks (global = 0, etc...)
} Compiler;

#define IN_A_LOCAL_SCOPE(compiler) ((compiler)->scopeDepth > 0)
#define IN_GLOBAL_SCOPE(compiler)  ((compiler)->scopeDepth == 0)

#define LOCAL_NOT_FOUND            (-1)
#define LOCAL_UNINITIALIZED        (-1)
#define LAST_LOCAL(compiler)       ((compiler)->locals[(compiler)->localCount - 1])

// Types of expressions in the grammar's expression-related rules. These types
// are returned up from the parser's recursive descent.
typedef enum expr_type {
  EXPR_BINARY,
  EXPR_GROUP,
  EXPR_LITERAL,
  EXPR_TERNARY,
  EXPR_UNARY,
  EXPR_VAR,
  EXPR_ERR,
} ExprType;

Parser parser;
Compiler *currentCompiler;

static void expression();
static void declaration();
static void statement();
static void varDecl();

static void emitByte(uint8_t byte);

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

static void initCompiler(Compiler *compiler) {
  compiler->localCount = 0;
  compiler->scopeDepth = 0;
  currentCompiler      = compiler;
}

static void beginScope() {
  currentCompiler->scopeDepth++;
}

static void endScope() {
  currentCompiler->scopeDepth--;

  // When exiting the scope, we need to remove the variable declarations from
  // the exiting scope, as well as cleanup the initializer values on the stack.
  while (currentCompiler->localCount > 0 &&
         LAST_LOCAL(currentCompiler).depth > currentCompiler->scopeDepth) {
    emitByte(OP_POP);
    currentCompiler->localCount--;
  }
}

static void markInitialized() {
  LAST_LOCAL(currentCompiler).depth = currentCompiler->scopeDepth;
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

static void consume(TokType type, const char *message) {
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
 * Emits the OP_LOOP instruction with a 2-byte operand. The operand specifies
 * the number of instructions to jump back - this calculated value includes
 * the jump of the 2-byte operand as well (count - loopStartOffset + 2).
 */
static void emitLoop(unsigned int loopStartOffset) {
  emitByte(OP_LOOP);

  Chunk *chunk        = currentChunk();
  int bytesToJumpBack = chunk->count - loopStartOffset + 2;

  if (bytesToJumpBack > UINT16_MAX) {
    errorPrev("loop body jump exceeded max of 65535 bytes to");
  }

  emitByte((bytesToJumpBack >> 8) & 0xFF); // High byte
  emitByte(bytesToJumpBack & 0xFF);        // Low byte
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

  if (bytesToJump > UINT16_MAX) {
    errorPrev("exceeded max of 65535 bytes to jump over");
  }

  chunk->code[offset]     = (bytesToJump >> 8) & 0xFF; // High byte
  chunk->code[offset + 1] = bytesToJump & 0xFF;        // Low byte
}

static unsigned int makeConstant(Value constant) {
  unsigned int constantIndex = appendConstant(currentChunk(), constant);

  // A byte can only refer to 256 different constant indexes.
  if (constantIndex > UINT8_MAX) {
    errorPrev("exceeded max of 256 constants in one chunk");
    return 0;
  }

  return constantIndex;
}

static void emitConstant(Value constant) {
  emitBytes(OP_CONSTANT, makeConstant(constant));
}

static uint8_t identifierConstant(Token *name) {
  ObjString *identifier = copyString(name->start, name->len);
  return makeConstant(OBJ_VAL(identifier));
}

static bool identifiersEqual(Token *a, Token *b) {
  return a->len == b->len && strncmp(a->start, b->start, a->len) == 0;
}

/*
 * Walk backwards of the compilers locals array to find the LAST declared
 * variable with the given name - then variable shadowing behaves correctly.
 * Returns the index of the resolved local from the compilers locals array,
 * otherwise -1 (macro `LOCAL_NOT_FOUND`). The resolved index corresponds with
 * the VM's stack slot index layout.
 */
static int resolveLocalVar(Compiler *compiler, Token *name) {
  for (int i = compiler->localCount - 1; i >= 0; i--) {
    Local *local = &compiler->locals[i];

    if (identifiersEqual(name, &local->name)) {
      if (local->depth == LOCAL_UNINITIALIZED) {
        errorPrev("can't read local variable in its own initializer");
      }

      return i;
    }
  }

  return LOCAL_NOT_FOUND;
}

static void addLocalVar(Token name) {
  if (currentCompiler->localCount >= UINT8_MAX + 1) {
    errorPrev("exceeded max of 256 local variables in function");
    return;
  }

  Local *local = &currentCompiler->locals[currentCompiler->localCount];
  local->name  = name;
  local->depth = LOCAL_UNINITIALIZED;

  currentCompiler->localCount++;
}

static void declareVariable() {
  // Global vars are late bound, so the compiler doesn't keep track of them.
  if (IN_GLOBAL_SCOPE(currentCompiler))
    return;

  Token *name = &parser.prev;

  // Check variable name is not redeclared in the same scope
  // Work backwards from the compilers locals to work from inner scope outwards
  for (int i = currentCompiler->localCount - 1; i >= 0; i--) {
    Local *local = &currentCompiler->locals[i];

    // Exit the loop early if we have checked all the locals in the compiler's
    // current scope depth.
    if (local->depth != LOCAL_UNINITIALIZED &&
        local->depth < currentCompiler->scopeDepth)
      break;

    if (identifiersEqual(name, &local->name)) {
      errorPrev("variable with this name already declared in this scope");
    }
  }

  addLocalVar(*name);
}

static void defineVariable(uint8_t identifierOffset) {
  // No runtime code needed to define a local variable as the temporary value
  // emitted from the initializer (or nil) is already on top of the stack.
  if (IN_A_LOCAL_SCOPE(currentCompiler)) {
    markInitialized();
    return;
  }

  emitBytes(OP_DEF_GLOBAL, identifierOffset);
}

static uint8_t parseVariable(const char *errorMessage) {
  consume(TOK_IDENTIFIER, errorMessage);

  declareVariable();
  if (IN_A_LOCAL_SCOPE(currentCompiler))
    return 0;

  return identifierConstant(&parser.prev);
}

static void emitReturn() {
  emitByte(OP_RETURN);
}

// Skip tokens until a statement boundary is reached
static void synchronize() {
  parser.panicMode = false;

  while (parser.cur.type != TOK_EOF) {
    if (parser.prev.type == TOK_SEMICOLON)
      return;

    switch (parser.cur.type) {
      case TOK_IF:
      case TOK_VAR:
      case TOK_PRINT: return;
      default:        advance();
    }
  }
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

static void variable() {
  // Only get the variable if it is not being assigned to
  if (check(TOK_EQ))
    return;

  Token *name = &parser.prev;

  int localVarIndex = resolveLocalVar(currentCompiler, name);

  if (localVarIndex == LOCAL_NOT_FOUND) {
    emitBytes(OP_GET_GLOBAL, identifierConstant(&parser.prev));
  } else {
    emitBytes(OP_GET_LOCAL, localVarIndex);
  }
}

static ExprType primary() {
  if (match(TOK_NUMBER)) {
    number();
    return EXPR_LITERAL;
  }

  if (match(TOK_STRING)) {
    string();
    return EXPR_LITERAL;
  }

  if (match(TOK_LEFT_PAREN)) {
    grouping();
    return EXPR_GROUP;
  }

  if (match(TOK_FALSE)) {
    emitByte(OP_FALSE);
    return EXPR_LITERAL;
  }

  if (match(TOK_TRUE)) {
    emitByte(OP_TRUE);
    return EXPR_LITERAL;
  }

  if (match(TOK_NIL)) {
    emitByte(OP_NIL);
    return EXPR_LITERAL;
  }

  if (match(TOK_IDENTIFIER)) {
    variable();
    return EXPR_VAR;
  }

  errorCur("expect expression");
  return EXPR_ERR;
}

static ExprType unary() {
  // allow recursive calls to itself for multiple prefixes e.g. "--5"
  if (match(TOK_BANG)) {
    unary();
    emitByte(OP_NOT);
    return EXPR_UNARY;
  }

  if (match(TOK_MINUS)) {
    unary();
    emitByte(OP_NEGATE);
    return EXPR_UNARY;
  }

  return primary();
}

static ExprType factor() {
  ExprType exprType = unary();

  while (match(TOK_STAR) || match(TOK_SLASH)) {
    TokType type = parser.prev.type;

    unary();

    switch (type) {
      case TOK_STAR:  emitByte(OP_MULTIPLY); break;
      case TOK_SLASH: emitByte(OP_DIVIDE); break;
      default:        break;
    }

    exprType = EXPR_BINARY;
  }

  return exprType;
}

static ExprType term() {
  ExprType exprType = factor();

  while (match(TOK_PLUS) || match(TOK_MINUS)) {
    TokType type = parser.prev.type;

    factor();

    switch (type) {
      case TOK_PLUS:  emitByte(OP_ADD); break;
      case TOK_MINUS: emitByte(OP_SUBTRACT); break;
      default:        break;
    }

    exprType = EXPR_BINARY;
  }

  return exprType;
}

static ExprType comparison() {
  ExprType exprType = term();

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

    exprType = EXPR_BINARY;
  }

  return exprType;
}

static ExprType equality() {
  ExprType exprType = comparison();

  while (match(TOK_EQ_EQ) || match(TOK_BANG_EQ)) {
    TokType type = parser.prev.type;

    comparison();

    switch (type) {
      case TOK_EQ_EQ:   emitByte(OP_EQ); break;
      case TOK_BANG_EQ: emitByte(OP_NOT_EQ); break;
      default:          break;
    }

    exprType = EXPR_BINARY;
  }

  return exprType;
}

static ExprType logicalAnd() {
  ExprType exprType = equality();

  while (match(TOK_AND)) {
    int operandOffset = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP); // if LHS falsy, pop removes it from stack
    equality();
    patchJump(operandOffset);
    exprType = EXPR_BINARY;
  }

  return exprType;
}

static ExprType logicalOr() {
  ExprType exprType = logicalAnd();

  while (match(TOK_OR)) {
    int operandOffset = emitJump(OP_JUMP_IF_TRUE);
    emitByte(OP_POP); // if LHS falsy, pop removes it from stack
    logicalAnd();
    patchJump(operandOffset);
    exprType = EXPR_BINARY;
  }

  return exprType;
}

static ExprType conditional() {
  // This statement will compile the condition already if parsing conditional.
  ExprType exprType = logicalOr();

  if (match(TOK_QUESTION)) {
    exprType = EXPR_TERNARY;

    // Then branch - jump over condition if false. Also emit a jump at the end
    // of the then branch to jump over the else branch if condition is true.
    int jumpToElseOperandOffset = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP); // pop condition
    expression();
    int jumpOverElseOperandOffset = emitJump(OP_JUMP);

    consume(TOK_COLON, "expect ':' after then branch of conditional");

    // Else branch - jump here if condition is false.
    patchJump(jumpToElseOperandOffset);
    emitByte(OP_POP); // pop condition
    conditional();
    patchJump(jumpOverElseOperandOffset);
  }

  return exprType;
}

static void assignment() {
  ExprType exprType = conditional();

  if (check(TOK_EQ)) {
    Token name = parser.prev;

    advance(); // consume '='

    switch (exprType) {
      case EXPR_VAR: {
        assignment();

        int localVarIndex = resolveLocalVar(currentCompiler, &name);

        if (localVarIndex == LOCAL_NOT_FOUND) {
          emitBytes(OP_SET_GLOBAL, identifierConstant(&name));
        } else {
          emitBytes(OP_SET_LOCAL, localVarIndex);
        }

        break;
      }
      default: errorPrev("invalid assignment target");
    }
  }
}

static void expression() {
  assignment();
}

static void blockStmt() {
  beginScope();

  while (!check(TOK_RIGHT_BRACE) && !check(TOK_EOF)) {
    declaration();
  }

  consume(TOK_RIGHT_BRACE, "expect '}' at end of block");

  endScope();
}

static void exprStmt() {
  expression();
  consume(TOK_SEMICOLON, "expect ';' at end");
  emitByte(OP_POP);
}

static void ifStmt() {
  // Compile the condition and leave it on the stack
  consume(TOK_LEFT_PAREN, "expect '(' after 'if'");
  expression();
  consume(TOK_RIGHT_PAREN, "expect ')' after 'if'");

  // Emit a OP_JUMP_IF_FALSE to jump past the true condition branch
  int ifFalseOperandOffset = emitJump(OP_JUMP_IF_FALSE);

  // True branch. No jump from OP_JUMP_IF_FALSE.
  // Pop the condition and compile the 'then' statement.
  emitByte(OP_POP);
  statement();

  // At the end of the 'then' statement, emit a OP_JUMP to jump over the 'else'.
  int endThenOperandOffset = emitJump(OP_JUMP);

  // Else branch - OP_JUMP_IF_FALSE jumps here.
  // Pop the condition and compile the 'else' statement (if any).
  patchJump(ifFalseOperandOffset);
  emitByte(OP_POP);

  if (match(TOK_ELSE)) {
    statement();
  }

  // Patch the end of the 'then' statement OP_JUMP to skip the 'else' branch.
  patchJump(endThenOperandOffset);
}

static void printStmt() {
  expression();
  consume(TOK_SEMICOLON, "expect ';' at end");
  emitByte(OP_PRINT);
}

static void whileStmt() {
  unsigned int conditionOffset = currentChunk()->count;

  // Condition
  consume(TOK_LEFT_PAREN, "expect '(' after 'while'");
  expression();
  consume(TOK_RIGHT_PAREN, "expect ')' after condition");

  int exitLoopOperandOffset = emitJump(OP_JUMP_IF_FALSE);

  // While statement body - pop the condition, compile body and emit the loop
  // bytecode to jump back to before the condition expression.
  emitByte(OP_POP);
  statement();
  emitLoop(conditionOffset);

  // Backpatch the exit loop jump, and pop the condition a final time
  patchJump(exitLoopOperandOffset);
  emitByte(OP_POP);
}

static void forStmt() {
#define NO_EXIT_LOOP_OFFSET (-1)
  beginScope();

  consume(TOK_LEFT_PAREN, "expect '(' after 'for'");

  // Initializer
  if (match(TOK_SEMICOLON)) {
    // No initializer
  } else if (match(TOK_VAR)) {
    varDecl();
  } else {
    exprStmt();
  }

  // Condition
  unsigned int loopStartOffset = currentChunk()->count;
  int exitLoopOperandOffset    = NO_EXIT_LOOP_OFFSET;

  if (!match(TOK_SEMICOLON)) {
    expression();
    consume(TOK_SEMICOLON, "expect ';' after 'for' loop condition");

    exitLoopOperandOffset = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP); // pop loop condition off stack when it is true
  }

  // Increment
  if (!match(TOK_RIGHT_PAREN)) {
    int incrementOperandOffset        = emitJump(OP_JUMP);
    unsigned int incrementStartOffset = currentChunk()->count;

    expression();
    emitByte(OP_POP); // pop evaluated expression off stack
    consume(TOK_RIGHT_PAREN, "expect ')' after 'for' loop clauses");

    // The increment runs after the body, so we need to jump back to the start
    // of the loop after the increment. After, reassign the loopStartOffset
    // so that when compiling the body next, it jumps back to the increment.
    emitLoop(loopStartOffset);
    loopStartOffset = incrementStartOffset;
    patchJump(incrementOperandOffset);
  }

  // Body - after compilation, jumps to start of increment. If there is no
  // increment jump goes back to the start of the loop at the condition.
  statement();
  emitLoop(loopStartOffset);

  // Patch the condition jump (if there was a condition)
  if (exitLoopOperandOffset != NO_EXIT_LOOP_OFFSET) {
    patchJump(exitLoopOperandOffset);
    emitByte(OP_POP); // pop loop condition off stack when it is false
  }

  endScope();

#undef NO_EXIT_LOOP_OFFSET
}

static void statement() {
  if (match(TOK_LEFT_BRACE)) {
    blockStmt();
    return;
  }

  if (match(TOK_IF)) {
    ifStmt();
    return;
  }

  if (match(TOK_PRINT)) {
    printStmt();
    return;
  }

  if (match(TOK_WHILE)) {
    whileStmt();
    return;
  }

  if (match(TOK_FOR)) {
    forStmt();
    return;
  }

  exprStmt();
}

static void varDecl() {
  uint8_t identifierIndex = parseVariable("expect variable name");

  if (match(TOK_EQ)) {
    expression();
  } else {
    emitByte(OP_NIL);
  }

  consume(TOK_SEMICOLON, "expect ';' after variable declaration");
  defineVariable(identifierIndex);
}

static void declaration() {
  if (match(TOK_VAR)) {
    varDecl();
  } else {
    statement();
  }

  if (parser.panicMode) {
    synchronize();
  }
}

static void endCompiler() {
  emitReturn();
}

bool compile(const char *source, Chunk *chunk) {
  Scanner scanner;
  initScanner(&scanner, source);

  Compiler compiler;
  initCompiler(&compiler);

  parser.scanner   = &scanner;
  parser.chunk     = chunk;
  parser.hadError  = false;
  parser.panicMode = false;

  advance();

  while (!match(TOK_EOF)) {
    declaration();
  }

  endCompiler();
  return !parser.hadError;
}
