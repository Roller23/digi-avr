void function(void) {
  int haha = 3;
  haha++;
}

int main(void) {
  int a = 0;
  int b = a + 5;
  if (b == a) {
    b++;
  } else {
    a++;
  }
  return 0;
}