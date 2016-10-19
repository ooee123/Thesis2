int shouldNotSee_global;
int should_global = 2;
int shouldSee_1;
int shouldSee_2 = shouldSee_1 = 3;
int shouldSee_3 = shouldNotSee_global;

int main() {
    int a, b, c, d, e;
    a = b = c = d = e = 0;
    a = b + 2;
    c = a;
    d = a + e;
    e = a + d;
    foo(a, e = b + c);
    if (e < 2)
        d = 11;
    return d;
}