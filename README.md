# ASBTL - A Stack Based Toy Language

A toy programming language heavily inspired by the ideas behind the Lox
language from [Crafting Interpreters](https://craftinginterpreters.com/).

- Bytecode intermediate representation
- Stack-based VM bytecode execution

## Information

### Syntax Grammar

Used for parsing the sequence of scanned tokens during bytecode compilation.

```text
program   : expression ;

expression: factor ( ('+' | '-') factor )* ;

factor    : unary ( ('*' | '/') unary )* ;

unary     : ('-' | '!') unary | primary ;

primary   : NUMBER
          | 'false' | 'true' | 'nil'
          | IDENTIFIER
          | '(' expression ')' ;

```

### Lexical Grammar

Used by the scanner to obtain the tokens from the initial input.

```text
NUMBER    : DIGIT+ ('.' DIGIT+)? ;
IDENTIFIER: ALPHA (ALPHA | DIGIT)* ;
ALPHA     : 'a' ... 'z' | 'A' ... 'Z' | '_' ;
DIGIT     : '0' ... '9' ;
```

## Contributing

To build, run `make`.

### E2E Tests

Located in [tests](./tests/) and are written with [Bats](https://bats-core.readthedocs.io/en/stable/index.html).
Run with `make tests`.

Tests should be added in the [tests/suites](./tests/suite/) folder, under a
relevant `.bats` file.

### Unit Tests

Located in [unittests](./unittests/) and are written with [minunit](https://github.com/bzgec/minunit/blob/master/README.md).
Run with `make unittests`.
