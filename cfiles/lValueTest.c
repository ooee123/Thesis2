int shouldNotSee_global;
int should_global = 2;
int shouldSee_1;
int shouldSee_2 = shouldSee_1 = 3;
int shouldSee_3 = shouldNotSee_global;

int main() {
    int a, b, c, d, e, f;
    a = b = c = d = e = 0;
    a = b + 2;
    f = 5 + c;
    d = a + e;
    e = a + d;
    c = a;
    foo(a, e = b + c);
    if (e < 2) {
        f = 12;
        b = f + d;
        d = 11;
    } else {
        b = f + d;
        f = b;
        b = 99;
    }
    bar(f);
    return d;
}