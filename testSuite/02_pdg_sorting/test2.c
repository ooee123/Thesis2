int main() {
   int a, b;
   a = 0;
   print(a);
   a = 12;
   b = a;
   a = 13;

   int c, d;
   c = 0;
   if (a < 5) {
      c = 2;
   }

   d = 0;
   if (a < 4) {
      d = 2;
   } else {
      d = 6;
   }

   int e, f;
   e = 0;
   f = 0;
   if (e < 5) {
      f = 7;
   } else {
      e = 2;
      f = 8;
   }
   print(e);

   return a + d + e;
}
