int main() {
   double d1 = 0.0;
   double *d2 = &d1;
   double **d3 = &d2, ***d4;
   d4 = &d3;
   return 0;
}
