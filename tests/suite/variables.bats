setup() {
  load '../test-helpers/common'
  _common_setup
}

teardown() {
  _common_teardown
}

@test "define global variable" {
  _run_asbtl "var x = true; print x;"
  assert_success
  assert_output "true"
}

@test "uninitialized variable defaults to nil" {
  _run_asbtl "var x; print x;"
  assert_success
  assert_output "nil"
}

@test "undefined variable gives runtime error" {
  _run_asbtl "print x;"
  assert_failure
  assert_output -p "runtime error: undefined variable 'x'"
}

@test "assign variable updates value" {
  _run_asbtl "var x; x = 10; print x;"
  assert_success
  assert_output "10"
}

@test "assigning to undefined variable gives runtime error" {
  _run_asbtl "x = 10;"
  assert_failure
  assert_output -p "runtime error: undefined variable 'x'"
}

@test "invalid assignment target gives error" {
  _run_asbtl "var a; var b; a * b = 10;"
  assert_failure
  assert_output -p "invalid assignment target"
}

@test "assignment right associativity" {
  _run_asbtl "var a; var b; var c; a = b = c = 3; print a + b + c;"
  assert_success
  assert_output "9"
}
