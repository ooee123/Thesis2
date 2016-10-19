#include <stdio.h>

int main() {
   int a, b, c;
   a = b = c = 1;
   b > 0 ? (a = 2) : (b = 3);
   printf("%d %d %d\n", a, b, c);
   return 0;
}
