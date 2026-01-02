int if_test(int a, int b) {
  int c;
  int t = 1;
  if (t > 3) {
    c = a;
  } else {
    c = b;
  }
  return c;
}


int loop() {
  int n = if_test(10, 50);
  int sum = 0;
  int i = 0;
  while (i < n) {
    sum = sum + i;
    i = i + 1;
  }
  return sum;
}

int main() {
  return loop();
}
