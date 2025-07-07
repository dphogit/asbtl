setup() {
  load '../test-helpers/common'
  _common_setup
}

teardown() {
  _common_teardown
}

@test "if return" {
  _run_asbtl '
  func f() { if (true) return 0; }
  print f();'

  assert_success
  assert_output "0"
}

@test "else return" {
  _run_asbtl '
  func f() { if (false) 1; else return 0; }
  print f();'

  assert_success
  assert_output "0"
}

@test "return conditional" {
  _run_asbtl '
  func f() { return true ? 0 : 1; }
  print f();'

  assert_success
  assert_output "0"
}

@test "while return" {
  _run_asbtl '
  func f() { while (true) return 0; }
  print f();'

  assert_success
  assert_output '0'
}

@test "no value returns nil" {
  _run_asbtl '
  func f() { return; }
  print f();
  '

  assert_success
  assert_output 'nil'
}

@test "no return statement returns nil" {
  _run_asbtl '
  func f() { }
  print f();
  '

  assert_success
  assert_output 'nil'
}

@test "return exits early" {
  _run_asbtl '
  func f() { return 0; return 1; }
  print f();
  '

  assert_success
  assert_output "0"
}

@test "return top level gives error" {
  _run_asbtl 'return 0;'

  assert_failure
  assert_output -p "error at 'return': can't return from top-level code"
}
