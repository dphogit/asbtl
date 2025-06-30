setup() {
  load '../test-helpers/common'
  _common_setup
}

teardown() {
  _common_teardown
}

@test "if truthy condition" {
  _run_asbtl "if (true) print 1;"
  assert_success
  assert_output "1"
}

@test "if falsy condition" {
  _run_asbtl "if (false) print 1;"
  assert_success
  refute_output "1"
}

@test "if with block" {
  _run_asbtl "if (true) { print 1; }"
  assert_success
  assert_output "1"
}

@test "if with assignment in condition" {
  _run_asbtl "var cond = false; if (cond = true) { print 1; }"
  assert_success
  assert_output "1"
}

@test "if else truthy condition" {
  _run_asbtl "if (true) print 1; else print 0;"
  assert_success
  assert_output "1"
}

@test "if else falsy condition" {
  _run_asbtl "if (false) print 1; else print 0;"
  assert_success
  assert_output "0"
}

@test "else with block" {
  _run_asbtl "if (false) nil; else { print 1; }"
  assert_success
  assert_output "1"
}
