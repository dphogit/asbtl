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

factor    : primary ( ('*' | '/') primary)* ;

primary   : NUMBER
          | BOOLEAN
          | '(' expression ')' ;
```

### Lexical Grammar

Used by the scanner to obtain the tokens from the initial input.

```text
NUMBER    : DIGIT+ ('.' DIGIT+)? ;
IDENTIFIER: ALPHA (ALPHA | DIGIT)* ;
BOOLEAN   : 'true' | 'false' ;
ALPHA     : 'a' ... 'z' | 'A' ... 'Z' | '_' ;
DIGIT     : '0' ... '9' ;
```

## Contributing

To build, run `make`.

### E2E Tests

Located in [tests](./tests/) and are written with [Bats](https://bats-core.readthedocs.io/en/stable/index.html).
Run with `make tests`.

Tests should be added in the [tests/suites](./tests/suite/) folder, under a
relevant category subfolder. This provides guidance on the projects way
of running the binary with an input file to write test cases against.

#### Example: Testing Addition

To test addition functionality, add the `suite/operators/add.asbtl` and
`suite/operators/add.bats` for the input ASBTL file and the file containing
the test cases.

```text
// add.asbtl
print 1 + 2
```

```bash
# add.bats
setup() {
  load '../../test-helpers/common'
  _common_setup
}

@test "add two numbers" {
  _run_file "add.asbtl"
  assert_success
  assert_output '3'
}
```

### Unit Tests

Located in [unittests](./unittests/) and are written with [minunit](https://github.com/bzgec/minunit/blob/master/README.md).
Run with `make unittests`.

Inspecting the test files should make writing new tests intuitive.
