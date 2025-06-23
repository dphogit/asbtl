setup() {
  load '../test-helpers/common'
  _common_setup
}

teardown() {
  _common_teardown
}

@test "addition" {
  _run_asbtl "1 + 2"
  assert_success
  assert_output "3"
}

@test "addition with non-number operand gives error" {
  _run_asbtl "1 + true"
  assert_failure
  assert_output -p 'operands must be numbers'
}

@test "subtraction" {
  _run_asbtl "2 - 1"
  assert_success
  assert_output "1"
}

@test "multiplication" {
  _run_asbtl "5 * 9"
  assert_success
  assert_output "45"
}

@test "division" {
  _run_asbtl "10 / 5"
  assert_success
  assert_output "2"
}

@test "multiple operators follows precedence" {
  _run_asbtl "5 + 5 * 10"
  assert_success
  assert_output "55"
}

@test "not false returns true" {
  _run_asbtl "!false"
  assert_success
  assert_output "true"
}

@test "not true returns false" {
  _run_asbtl "!true"
  assert_success
  assert_output "false"
}

@test "not nil returns true" {
  _run_asbtl "!nil"
  assert_success
  assert_output "true"
}

@test "not truthy value returns false" {
  _run_asbtl "!21"
  assert_success
  assert_output "false"
}

@test "negation" {
  _run_asbtl "-21"
  assert_success
  assert_output "-21"
}

@test "negation double" {
  _run_asbtl "--21"
  assert_success
  assert_output "21"
}

@test "negation with non-number operand errors" {
  _run_asbtl "-true"
  assert_failure
  assert_output -p "negation operand must be a number"
}
