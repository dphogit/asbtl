setup() {
  load '../../test-helpers/common'
  _common_setup
}

@test "expression with addition and multiplication" {
  _run_file "precedence.asbtl"
  assert_success
  assert_output '55'
}
