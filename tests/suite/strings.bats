setup() {
  load '../test-helpers/common'
  _common_setup
}

teardown() {
  _common_teardown
}

@test "string literal" {
  _run_asbtl 'print "Hello, World!";'
  assert_success
  assert_output "Hello, World!"
}

@test "concatenate two strings" {
  _run_asbtl 'print "foo" + "bar";'
  assert_success
  assert_output "foobar"
}

@test "concatenate three strings" {
  _run_asbtl 'print "foo" + "bar" + "baz";'
  assert_success
  assert_output "foobarbaz"
}

@test "unterminated string" {
  _run_asbtl '"'
  assert_failure
  assert_output -p "error: unterminated string"
}

@test "string spans multiple lines" {
  _run_asbtl 'print "foo\nbar";'
  assert_success
  assert_output "foo\nbar"
}

@test "string value equality returns true" {
  _run_asbtl 'print "a" == "a";'
  assert_success
  assert_output "true"
}

@test "string different values returns false" {
  _run_asbtl 'print "a" == "b";'
  assert_success
  assert_output "false"
}
