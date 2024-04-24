int foo(int a, int b) {
  return a + b;
}


int call_test(int a, int b) {
  int c = foo(a+b, a);
  return c;
}
