# ASBTL - A Stack Based Toy Language

A toy programming language heavily inspired by the ideas behind the Lox
language from [Crafting Interpreters](https://craftinginterpreters.com/).

- Bytecode intermediate representation
- Stack-based VM bytecode execution

## Information

### Syntax Grammar

Used for parsing the sequence of scanned tokens during bytecode compilation.

```text
program    : declaration* EOF ;

declaration: varDecl
           | statement

varDecl    : 'var' IDENTIFIER ( '=' expression )? ';' ;

statement  : blockStmt
           | exprStmt
           | forStmt
           | ifStmt
           | printStmt 
           | whileStmt ;

blockStmt  : '{' declaration* '}' ;

exprStmt   : expression ';' ;

forStmt    : 'for' '(' ( varDecl | exprStmt | ';' )
                       expression? ';'
                       expression? ')' statement ;

ifStmt     : 'if' '(' expression ')' statement ( 'else' statement )? ;

printStmt  : 'print' expression ';' ;

whileStmt  : 'while' '(' expression ')' statement ';' ;

expression : assignment ;

assignment : IDENTIFIER '=' assignment
           | or ;

or         : and ( '||' and )* ;

and        : equality ( '&&' equality )* ;

equality   : comparison ( ( '==' | '!=' ) comparison )* ;

comparison : term ( ( '<' | '<=' | '>' | '>=' ) term )* ;

term       : factor ( ( '+' | '-' ) factor )* ;

factor     : unary ( ( '*' | '/' ) unary )* ;

unary      : (( '-' | '!' ) unary ) | primary ;

primary    : NUMBER
           | STRING
           | 'false' | 'true' | 'nil'
           | IDENTIFIER
           | '(' expression ')' ;
```

### Lexical Grammar

Used by the scanner to obtain the tokens from the initial input.

```text
NUMBER    : DIGIT+ ('.' DIGIT+)? ;
STRING    : '"' [any character except '"'] '"' ;
IDENTIFIER: ALPHA (ALPHA | DIGIT)* ;
ALPHA     : 'a' ... 'z' | 'A' ... 'Z' | '_' ;
DIGIT     : '0' ... '9' ;
```

## Contributing

To build, run `make`. Run the binary with `make run`.

### E2E Tests

Located in [tests](./tests/) and are written with [Bats](https://bats-core.readthedocs.io/en/stable/index.html).
Run with `make tests`.

Tests should be added in the [tests/suites](./tests/suite/) folder, under a
relevant `.bats` file.

### Unit Tests

Located in [unittests](./unittests/) and are written with [minunit](https://github.com/bzgec/minunit/blob/master/README.md).
Run with `make unittests`.
