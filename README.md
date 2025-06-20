# ASBTL - A Stack Based Toy Language

A toy programming language heavily inspired by the ideas behind the Lox
language from [Crafting Interpreters](https://craftinginterpreters.com/).

- Bytecode intermediate representation
- Stack-based VM bytecode execution

## Grammar

The grammars used for ASBTL.

### Syntax Grammar

Used for parsing the sequence of scanned tokens.

```text
program   : expression

expression: NUMBER ( ('+' | '-') NUMBER )* ;
```

### Lexical Grammar

Used by the scanner to obtain the tokens from the initial input.

```text
NUMBER: DIGIT+ ('.' DIGIT+)? ;
DIGIT : '0' ... '9'
```

## Tests

Unit tests are written with [minunit](https://github.com/bzgec/minunit/blob/master/README.md)
