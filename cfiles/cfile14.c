#include <stdio.h>

int main() {
    int a, b, c, d, e;
    a = 0;
    b = 1;
    c = 0;
    d = 0;
    e = 12;
    a = b > 0 ? c = 0, d = -1 : e;

    printf("%d, %d, %d, %d, %d", a, b, c, d, e);
    return 0;
}

/* If b <= 0, -> 12,  0, 0,  0, 12
   If b > 1,  -> -1, -1, 0, -1, 12
   */