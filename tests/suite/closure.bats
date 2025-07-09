setup() {
  load '../test-helpers/common'
  _common_setup
}

teardown() {
  _common_teardown
}

@test "open upvalue closure" {
  _run_asbtl "
  { 
    var a = true;
    
    func f() {
      print a;
    }

    f();
  }"

  assert_success
  assert_output "true"
}

@test "closed upvalue closure" {
  _run_asbtl '
  var outer;

  {
    var local = "local";

    func inner() {
      print local;
    }

    outer = inner;
  }

  outer();'

  assert_success
  assert_output "local"
}

@test "counter closure" {
  _run_asbtl "
  func makeCounter() {
    var count = 0;

    func increment() {
      count = count + 1;
      print count;
    }

    return increment;
  }

  var counter = makeCounter();

  counter(); // 1
  counter(); // 2
  counter(); // 3
  "

  assert_success
  assert_line -n 0 "1"
  assert_line -n 1 "2"
  assert_line -n 2 "3"
}

@test "assign to shadowed later" {
  _run_asbtl '
  var a = "global";

  {
    func assign() {
      a = "assigned";
    }

    var a = "inner";
    assign();
    print a; // expect "inner"
  }

  print a; // expect "assigned"'

  assert_success
  assert_line -n 0 "inner"
  assert_line -n 1 "assigned"
}

@test "close over function parameter" {
  _run_asbtl '
  var f;

  func foo(param) {
    func inner() {
      print param;
    }

    f = inner;
  }

  foo("param");
  f(); // expect "param"'

  assert_success
  assert_output "param"
}

@test "reuse closure slot" {
  _run_asbtl '
  {
    var f;

    {
      var a = "a";
      func printA() { print a; }
      f = printA;
    }

    {
      // As a is out of scope, local slot should be reused by b.
      // Make sure f still closes over a.
      var b = "b";
      f(); // expect "a"
    }
  }'

  assert_success
  assert_output "a"
}
