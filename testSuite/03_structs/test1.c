int main() {
    int a;
    int b;
    int c;
    int d;
    int *aa, *bb, *cc, *dd;
    aa = &a;
    bb = &b;
    cc = &c;
    dd = &d;
    a = b = c = d = 0;
    *aa = 12;

    a = 14;
    *bb = 14 + a;
    c = *bb + 4;
    c = a + a;
    print(a);
    for (int e = 0; e < d; e++) {
       *aa = 7;
    }
    a = b;
    print(a);
    return a + b + c + d + *aa + *bb + *cc + *dd;
}
