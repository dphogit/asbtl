setup() {
  load '../../test-helpers/common'
  _common_setup
}

@test "add two numbers" {
  _run_file "add.asbtl"
  assert_success
  assert_output '3'
}
