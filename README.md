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

### E2E Tests

Located in [tests](./tests/) and are written with [Bats](https://bats-core.readthedocs.io/en/stable/index.html).

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

Inspecting the test files should make writing new tests intuitive.
