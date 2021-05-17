#include <stdio.h>

int main() {
    int i = 6;

    if (i != 0) { printf("%d != 0\n", i); }
    if (i < 0) printf("false\n");
    else printf("true\n");
    { int div = 6; if (i % div == 0) printf("%d divisible by %d\n", i, div); }
    { int div = 3; if (i % div == 0) printf("%d divisible by %d\n", i, div); }
    { int div = 2; if (i % div == 0) printf("%d divisible by %d\n", i, div); }
    { int div = 5; if (i % div == 0) printf("%d divisible by %d\n", i, div); }

    return 0;
}
