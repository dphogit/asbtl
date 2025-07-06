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

@test "while loop single expression body" {
  _run_asbtl "var i = 0; while (i < 3) print i = i + 1;"
  assert_success
  assert_line -n 0 '1'
  assert_line -n 1 '2'
  assert_line -n 2 '3'
}

@test "while loop block body" {
  _run_asbtl "var i = 0; while (i < 3) { print i + 1; i = i + 1; }"
  assert_success
  assert_line -n 0 '1'
  assert_line -n 1 '2'
  assert_line -n 2 '3'
}

@test "for loop single expression body" {
  _run_asbtl "for (var i = 0; i < 3; i = i + 1) print i + 1;"
  assert_success
  assert_line -n 0 '1'
  assert_line -n 1 '2'
  assert_line -n 2 '3'
}

@test "for loop block body" {
  _run_asbtl "for (var i = 0; i < 3; i = i + 1) { print i + 1; }"
  assert_success
  assert_line -n 0 '1'
  assert_line -n 1 '2'
  assert_line -n 2 '3'
}

@test "for loop variable defined and initialized before" {
  _run_asbtl "var i = 0; for (i = 0; i < 3; i = i + 1) { print i + 1; }"
  assert_success
  assert_line -n 0 '1'
  assert_line -n 1 '2'
  assert_line -n 2 '3'
}

@test "for loop no condition clause" {
  skip # TODO: Implement break/continue statements once scope is introduced
  _run_asbtl "for (var i = 0; ; i = i + 1) { print i + 1; if (i == 2) break; }"
  assert_success
  assert_line -n 0 '1'
  assert_line -n 1 '2'
  assert_line -n 2 '3'
}

@test "for loop no increment clause" {
  _run_asbtl "for (var i = 0; i < 3; ) { i = i + 1; print i; }"
  assert_success
  assert_line -n 0 '1'
  assert_line -n 1 '2'
  assert_line -n 2 '3'
}

@test "conditional true branch" {
  _run_asbtl "var x = 2; print x == 2 ? 1 : 0;"
  assert_success
  assert_output "1"
}

@test "conditional false branch" {
  _run_asbtl "var x = 3; print x == 2 ? 1 : 0;"
  assert_success
  assert_output "0"
}

@test "conditional long chain" {
  _run_asbtl "print false ? 1 : false ? 2 : false ? 3 : 4;"
  assert_success
  assert_output "4"
}

@test "conditional nested" {
  _run_asbtl "print true ? false ? 1 : 2 : 3;"
  assert_success
  assert_output "2"
}

@test "conditional expression in then branch" {
  _run_asbtl "print true ? 1 + 2 : 7;"
  assert_success
  assert_output "3"
}

@test "conditional expression in false branch" {
  _run_asbtl "print false ? 1 : 3 + 4;"
  assert_success
  assert_output "7"
}

@test "conditional used inside expression" {
  _run_asbtl "print (true ? 2 : 3) + 4;"
  assert_success
  assert_output "6"
}

@test "conditional respects higher precedence operators" {
  # (true || false) => true, true ? 1 : 2 => 1
  _run_asbtl "print true || false ? 1 : 2;"
  assert_success
  assert_output "1"
}

@test "conditional missing colon gives error" {
  _run_asbtl "print true ? 1;"
  assert_failure
  assert_output -p "expect ':' after then branch of conditional"
}

@test "conditional missing else branch" {
  _run_asbtl "print true ? 1: ;"
  assert_failure
  assert_output -p "expect expression"
}
