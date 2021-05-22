#include <stdio.h>

char *p;
int a, b, c, d;

int main() {
    printf("a: %d, b: %d, c: %d, d: %d\n", a, b, c, d);
    a = 1, b = 2, c = 3, d = 4;
    printf("a: %d, b: %d, c: %d, d: %d\n", a, b, c, d);
    {
        int a = 10, b = 20, c = 30, d = 40;
        printf("a: %d, b: %d, c: %d, d: %d\n", a, b, c, d);
    }
    printf("a: %d, b: %d, c: %d, d: %d\n", a, b, c, d);
    return 0;
}
