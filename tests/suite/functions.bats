setup() {
  load '../test-helpers/common'
  _common_setup
}

teardown() {
  _common_teardown
}

@test "print user defined function" {
  _run_asbtl '
  func hello() { print "hello"; }
  print hello;'

  assert_success
  assert_output "<fn hello>"
}

@test "print native function" {
  _run_asbtl 'print clock;'
  assert_success
  assert_output "<native fn>"
}

@test "empty body" {
  _run_asbtl "
  func f() {}
  print f();"

  assert_success
  assert_output "nil"
}

@test "recursion" {
  _run_asbtl "
  func fib(n) { if (n < 2) return n; return fib(n - 1) + fib(n - 2); }
  print fib(8);"

  assert_success
  assert_output "21"
}

@test "mutual recursion" {
  _run_asbtl "
  func isEven(n) { return n == 0 ? true : isOdd(n - 1); }
  func isOdd(n) { return n == 0 ? false : isEven(n - 1); }

  print isEven(4);
  print isEven(3);
  print isOdd(3);
  print isOdd(4);"

  assert_success
  assert_line -n 0 "true"
  assert_line -n 1 "false"
  assert_line -n 2 "true"
  assert_line -n 3 "false"
}

@test "first class function" {
  _run_asbtl "
  func printArg(arg) { print arg; }
  func callFuncOnArg(f, arg) { f(arg); }

  callFuncOnArg(printArg, 0);
  "

  assert_success
  assert_output "0"
}

@test "body must be block" {
  _run_asbtl "func f() 1;"
  assert_failure
  assert_output -p "expect '{' at start of function body"
}

@test "too many arguments gives error" {
  _run_asbtl "
  func add(x, y) { return x + y; }
  print add(1, 2, 3);"

  assert_failure
  assert_output -p "expected 2 arguments, but got 3"
}

@test "missing arguments gives error" {
  _run_asbtl "
  func add(x, y) { return x + y; }
  print add(1);"

  assert_failure
  assert_output -p "expected 2 arguments, but got 1"
}

@test "too many parameters gives error" {
  _run_asbtl "
  func f(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15,
  p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31,
  p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47,
  p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58, p59, p60, p61, p62, p63,
  p64, p65, p66, p67, p68, p69, p70, p71, p72, p73, p74, p75, p76, p77, p78, p79,
  p80, p81, p82, p83, p84, p85, p86, p87, p88, p89, p90, p91, p92, p93, p94, p95,
  p96, p97, p98, p99, p100, p101, p102, p103, p104, p105, p106, p107, p108, p109,
  p110, p111, p112, p113, p114, p115, p116, p117, p118, p119, p120, p121, p122,
  p123, p124, p125, p126, p127, p128, p129, p130, p131, p132, p133, p134, p135,
  p136, p137, p138, p139, p140, p141, p142, p143, p144, p145, p146, p147, p148,
  p149, p150, p151, p152, p153, p154, p155, p156, p157, p158, p159, p160, p161,
  p162, p163, p164, p165, p166, p167, p168, p169, p170, p171, p172, p173, p174,
  p175, p176, p177, p178, p179, p180, p181, p182, p183, p184, p185, p186, p187,
  p188, p189, p190, p191, p192, p193, p194, p195, p196, p197, p198, p199, p200,
  p201, p202, p203, p204, p205, p206, p207, p208, p209, p210, p211, p212, p213,
  p214, p215, p216, p217, p218, p219, p220, p221, p222, p223, p224, p225, p226,
  p227, p228, p229, p230, p231, p232, p233, p234, p235, p236, p237, p238, p239,
  p240, p241, p242, p243, p244, p245, p246, p247, p248, p249, p250, p251, p252,
  p253, p254, p255) {}"

  assert_failure
  assert_output -p "can't have more than 255 parameters"
}

@test "missing comma in parameters gives error" {
  _run_asbtl "func f(a b) {}"
  assert_failure
  assert_output -p "expect ')' after function parameters"
}

@test "repeated parameter name gives error" {
  _run_asbtl "func f(a, a) {}"
  assert_failure
  assert_output -p "variable with this name already declared in this scope"
}

@test "no name gives error" {
  _run_asbtl "func (a, b) {}"
  assert_failure
  assert_output -p "expect function name"
}

@test "calling non-function gives error" {
  _run_asbtl "var f = 3; f();"
  assert_failure
  assert_output -p "can only call functions"
}
