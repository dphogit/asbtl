setup() {
  load '../test-helpers/common'
  _common_setup
}

teardown() {
  _common_teardown
}

@test "empty block" {
  _run_asbtl "{}"
  assert_success
}

@test "block with no closing brace" {
  _run_asbtl "{"
  assert_failure
  assert_output -p "expect '}' at end of block"
}

@test "statement executes inside block" {
  _run_asbtl "{ print true; }"
  assert_success
  assert_output "true"
}
