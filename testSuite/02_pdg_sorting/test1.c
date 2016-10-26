int main() {
   int a, b, c, d, e;
   c = 0;
   b = 1;
   a = 12;
   d = c + b;
   func(a);
   func2(b, d);
   b = a;
   a = 11;
   func3(c);
   e = a;
   a = 13;
   e = func4(b);
   c = 0;
   return a + b + c;
}
