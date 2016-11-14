#include <stdio.h>

void modifyingFunction(int *aa) {
   *aa = 2;
}

int nonModifyingFunction(int i) {
   return i * i;
}

int main() {
   int a;
   int *bb;
   a = 7;
   bb = &a;
   print(a);
   modifyingFunction(*a);
   print(a);
   a = 8;
   modifyingFunction(bb);
   print(a);
   nonModifyingFunction(a);
   return a;
}
