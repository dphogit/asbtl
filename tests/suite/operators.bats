setup() {
  load '../test-helpers/common'
  _common_setup
}

teardown() {
  _common_teardown
}

@test "addition" {
  _run_asbtl "print 1 + 2;"
  assert_success
  assert_output "3"
}

@test "addition with non-number operand gives error" {
  _run_asbtl "print 1 + true;"
  assert_failure
  assert_output -p 'operands must both be strings or both be numbers'
}

@test "subtraction" {
  _run_asbtl "print 2 - 1;"
  assert_success
  assert_output "1"
}

@test "multiplication" {
  _run_asbtl "print 5 * 9;"
  assert_success
  assert_output "45"
}

@test "division" {
  _run_asbtl "print 10 / 5;"
  assert_success
  assert_output "2"
}

@test "multiple operators follows precedence" {
  _run_asbtl "print 5 + 5 * 10;"
  assert_success
  assert_output "55"
}

@test "not false returns true" {
  _run_asbtl "print !false;"
  assert_success
  assert_output "true"
}

@test "not true returns false" {
  _run_asbtl "print !true;"
  assert_success
  assert_output "false"
}

@test "not nil returns true" {
  _run_asbtl "print !nil;"
  assert_success
  assert_output "true"
}

@test "not truthy value returns false" {
  _run_asbtl "print !21;"
  assert_success
  assert_output "false"
}

@test "negation" {
  _run_asbtl "print -21;"
  assert_success
  assert_output "-21"
}

@test "negation double" {
  _run_asbtl "print --21;"
  assert_success
  assert_output "21"
}

@test "negation with non-number operand errors" {
  _run_asbtl "print -true;"
  assert_failure
  assert_output -p "negation operand must be a number"
}

@test "less than a < b returns true" {
  _run_asbtl "print 1 < 2;"
  assert_success
  assert_output "true"
}

@test "less than a == b returns false" {
  _run_asbtl "print 2 < 2;"
  assert_success
  assert_output "false"
}

@test "less than a > b returns false" {
  _run_asbtl "print 3 < 2;"
  assert_success
  assert_output "false"
}

@test "less than equal a < b returns true" {
  _run_asbtl "print 1 <= 2;"
  assert_success
  assert_output "true"
}

@test "less than equal a == b returns true" {
  _run_asbtl "print 2 <= 2;"
  assert_success
  assert_output "true"
}

@test "less than equal a > b returns false" {
  _run_asbtl "print 3 <= 2;"
  assert_success
  assert_output "false"
}

@test "greater than equal a < b returns false" {
  _run_asbtl "print 1 >= 2;"
  assert_success
  assert_output "false"
}

@test "greater than equal a == b returns true" {
  _run_asbtl "print 2 >= 2;"
  assert_success
  assert_output "true"
}

@test "greater than equal a > b returns true" {
  _run_asbtl "print 3 >= 2;"
  assert_success
  assert_output "true"
}

@test "greater than a < b returns false" {
  _run_asbtl "print 1 > 2;"
  assert_success
  assert_output "false"
}

@test "greater than a == b returns false" {
  _run_asbtl "print 2 > 2;"
  assert_success
  assert_output "false"
}

@test "greater than a > b returns true" {
  _run_asbtl "print 3 > 2;"
  assert_success
  assert_output "true"
}

@test "equal a == b returns true" {
  _run_asbtl "print 2 == 2;"
  assert_success
  assert_output "true"
}

@test "equal a != b returns false" {
  _run_asbtl "print 2 == 3;"
  assert_success
  assert_output "false"
}

@test "not equal a == b returns false" {
  _run_asbtl "print 2 != 2;"
  assert_success
  assert_output "false"
}

@test "not equal a != b returns true" {
  _run_asbtl "print 2 != 3;"
  assert_success
  assert_output "true"
}

@test "false or false returns false" {
  _run_asbtl "print false || false;"
  assert_success
  assert_output "false"
}

@test "false or true returns true" {
  _run_asbtl "print false || true;"
  assert_success
  assert_output "true"
}

@test "true or false returns true" {
  _run_asbtl "print true || false;"
  assert_success
  assert_output "true"
}

@test "true or true returns true" {
  _run_asbtl "print true || true;"
  assert_success
  assert_output "true"
}

@test "false and true returns false" {
  _run_asbtl "print false && true;"
  assert_success
  assert_output "false"
}

@test "false and false returns false" {
  _run_asbtl "print false && false;"
  assert_success
  assert_output "false"
}

@test "true and false returns false" {
  _run_asbtl "print true && false;"
  assert_success
  assert_output "false"
}

@test "true and true returns true" {
  _run_asbtl "print true && true;"
  assert_success
  assert_output "true"
}
