#include "scanner.h"
#include "token.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>

// Moves the scanner current position one forward, returning the recent char
static char advance(Scanner *scanner) {
  scanner->cur++;
  scanner->col++;
  return scanner->cur[-1];
}

static bool isAtEnd(Scanner *scanner) {
  return *scanner->cur == '\0';
}

static char peek(Scanner *scanner) {
  return *scanner->cur;
}

static char peekNext(Scanner *scanner) {
  return isAtEnd(scanner) ? '\0' : scanner->cur[1];
}

// Conditionally advance the scanner if current char matches the given
static bool match(Scanner *scanner, char c) {
  if (*scanner->cur == c) {
    advance(scanner);
    return true;
  }

  return false;
}

static bool isAlpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static Token token(Scanner *scanner, TokType type) {
  Token tok;
  tok.type  = type;
  tok.start = scanner->start;
  tok.len   = scanner->cur - scanner->start;
  tok.col   = scanner->col;
  tok.line  = scanner->line;
  return tok;
}

static Token error(Scanner *scanner, const char *msg) {
  Token tok;
  tok.type  = TOK_ERR;
  tok.start = msg;
  tok.len   = strlen(msg);
  tok.col   = scanner->col;
  tok.line  = scanner->line;
  return tok;
}

static Token number(Scanner *scanner) {
#define CONSUME_DIGITS()         \
  while (isdigit(peek(scanner))) \
  advance(scanner)

  CONSUME_DIGITS();

  // Look for fractional part of number
  if (peek(scanner) == '.') {
    if (!isdigit(peekNext(scanner)))
      return error(scanner, "expect digits after '.'");

    advance(scanner); // Consume '.'
    CONSUME_DIGITS();
  }

  return token(scanner, TOK_NUMBER);

#undef CONSUME_DIGITS
}

static void skipWhitespace(Scanner *scanner) {
#define CONSUME_INLINE_COMMENT()                     \
  while (peek(scanner) != '\n' && !isAtEnd(scanner)) \
  advance(scanner)

  while (true) {
    switch (peek(scanner)) {
      case '/':
        if (peekNext(scanner) == '/') {
          CONSUME_INLINE_COMMENT();
          continue;
        }
        return;
      case '\n':
        advance(scanner);
        scanner->line++;
        scanner->col = 1;
        continue;
      case ' ':
      case '\t':
      case '\r': advance(scanner); continue;
      default:   return;
    }
  }

#undef CONSUME_INLINE_COMMENT
}

static TokType checkKeyword(Scanner *scanner, int start, int len,
                            const char *rest, TokType type) {
  bool sameLen      = scanner->cur - scanner->start == start + len;
  bool sameContents = strncmp(scanner->start + start, rest, len) == 0;
  return sameLen && sameContents ? type : TOK_IDENTIFIER;
}

static TokType identifierType(Scanner *scanner) {
  // Use a trie to find if it is a language keyword or identifier
  switch (scanner->start[0]) {
    case 'e': return checkKeyword(scanner, 1, 3, "lse", TOK_ELSE);
    case 'f':
      if (scanner->cur - scanner->start > 1) {
        switch (scanner->start[1]) {
          case 'a': return checkKeyword(scanner, 2, 3, "lse", TOK_FALSE);
          case 'o': return checkKeyword(scanner, 2, 1, "r", TOK_FOR);
        }
      }
      break;
    case 'i': return checkKeyword(scanner, 1, 1, "f", TOK_IF);
    case 'n': return checkKeyword(scanner, 1, 2, "il", TOK_NIL);
    case 'p': return checkKeyword(scanner, 1, 4, "rint", TOK_PRINT);
    case 't': return checkKeyword(scanner, 1, 3, "rue", TOK_TRUE);
    case 'v': return checkKeyword(scanner, 1, 2, "ar", TOK_VAR);
    case 'w': return checkKeyword(scanner, 1, 4, "hile", TOK_WHILE);
  }

  return TOK_IDENTIFIER;
}

static Token identifier(Scanner *scanner) {
  char c = peek(scanner);
  while (isAlpha(c) || isdigit(c)) {
    advance(scanner);
    c = peek(scanner);
  }

  return token(scanner, identifierType(scanner));
}

static Token string(Scanner *scanner) {
  while (peek(scanner) != '"' && !isAtEnd(scanner)) {
    if (peek(scanner) == '\n') {
      scanner->line++; // allow strings to span multiple lines
    }

    advance(scanner);
  }

  if (isAtEnd(scanner))
    return error(scanner, "unterminated string");

  advance(scanner); // consume closing '"'

  return token(scanner, TOK_STRING);
}

void initScanner(Scanner *scanner, const char *source) {
  scanner->start = source;
  scanner->cur   = source;
  scanner->line  = 1;
  scanner->col   = 1;
}

Token scanNext(Scanner *scanner) {
  skipWhitespace(scanner);
  scanner->start = scanner->cur;

  if (isAtEnd(scanner))
    return token(scanner, TOK_EOF);

  char c = advance(scanner);

  if (isdigit(c))
    return number(scanner);

  if (isAlpha(c)) {
    return identifier(scanner);
  }

  switch (c) {
    case '+': return token(scanner, TOK_PLUS);
    case '-': return token(scanner, TOK_MINUS);
    case '*': return token(scanner, TOK_STAR);
    case '/': return token(scanner, TOK_SLASH);
    case '(': return token(scanner, TOK_LEFT_PAREN);
    case ')': return token(scanner, TOK_RIGHT_PAREN);
    case '{': return token(scanner, TOK_LEFT_BRACE);
    case '}': return token(scanner, TOK_RIGHT_BRACE);
    case ';': return token(scanner, TOK_SEMICOLON);
    case ':': return token(scanner, TOK_COLON);
    case '"': return string(scanner);
    case '?': return token(scanner, TOK_QUESTION);
    case '=': return token(scanner, match(scanner, '=') ? TOK_EQ_EQ : TOK_EQ);
    case '!':
      return token(scanner, match(scanner, '=') ? TOK_BANG_EQ : TOK_BANG);
    case '<':
      return token(scanner, match(scanner, '=') ? TOK_LESS_EQ : TOK_LESS);
    case '>':
      return token(scanner, match(scanner, '=') ? TOK_GREATER_EQ : TOK_GREATER);
    case '|':
      return match(scanner, '|')
                 ? token(scanner, TOK_OR)
                 : error(scanner, "'|' not supported. Did you mean '||'?");
    case '&':
      return match(scanner, '&')
                 ? token(scanner, TOK_AND)
                 : error(scanner, "'&' not supported. Did you mean '&&'?");
  }

  return error(scanner, "unexpected character");
}
