int main() {
   int a, b, c, d, e;
   a = b = c = d = e = 0;
   for (int i = 0; i < e && b < e; i++) {
      b = b + 1;
      e = e * a;
      d = b + a;
   }
   return a + c + e;
}
