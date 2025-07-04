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

@test "undefined global variable gives runtime error" {
  _run_asbtl "x;"
  assert_failure
  assert_output -p "runtime error: undefined variable 'x'"
}

@test "undefined local variable gives runtime error" {
  _run_asbtl "{ x; }"
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

@test "redeclare global variable" {
  _run_asbtl "var a = 1; var a; print a;"
  assert_success
  assert_output "nil"
}

@test "redefine global variable" {
  _run_asbtl "var a = 1; var a = 2; print a;"
  assert_success
  assert_output "2"
}

@test "local variable same scope level different block" {
  _run_asbtl "{ var a = 1; print a; } { var a = 2; print a; }"
  assert_success
  assert_line -n 0 "1"
  assert_line -n 1 "2"
}

@test "shadow global" {
  _run_asbtl 'var a = "global"; { var a = "local"; print a; } print a;'
  assert_success
  assert_line -n 0 "local"
  assert_line -n 1 "global"
}

@test "shadow local" {
  _run_asbtl '{ var a = "local"; { var a = "shadow"; print a; } print a; }'
  assert_success
  assert_line -n 0 "shadow"
  assert_line -n 1 "local"
}

@test "shadow and local" {
  _run_asbtl '{ var a = "outer"; { print a; var a = "inner"; print a; } }'
  assert_success
  assert_line -n 0 "outer"
  assert_line -n 1 "inner"
}

@test "use global in its own initializer" {
  _run_asbtl 'var a = "outer"; var a = a; print a;'
  assert_success
  assert_output "outer"
}

@test "use local in its own initializer gives error" {
  _run_asbtl 'var a = "outer"; { var a = a; }'
  assert_failure
  assert_output -p "can't read local variable in its own initializer"
}

@test "duplicate local declarations gives error" {
  _run_asbtl "{ var a = 1; var a = 2; }"
  assert_failure
  assert_output -p "variable with this name already declared in this scope"
}

@test "use keyword as variable identifier gives error" {
  _run_asbtl 'var false = "value";'
  assert_failure
  assert_output -p "expect variable name"
}

@test "compose local variables" {
  _run_asbtl '{ var a = "a"; var b = a + " b"; var c = b + " c"; print a; print b; print c; }'
  assert_success
  assert_line -n 0 "a"
  assert_line -n 1 "a b"
  assert_line -n 2 "a b c"
}
