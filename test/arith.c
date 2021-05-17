#include <stdio.h>

int main() {
    // arith
    printf("%d\n", 0);
    printf("%d\n", 31 + 15);
    printf("%d\n", 145 - 199);
    printf("%d\n", 324 * 71);
    printf("%d\n", 56 / 8);
    printf("%d\n", 54 % 8);
    printf("%d\n", 1 + 2 + 3);
    printf("%d\n", 1 - 2 - 3);
    printf("%d\n", 64 / 4 / 4);
    printf("%d\n", 64 / (4 / 4));
    printf("%d\n", 14 + 12 * 3 - 4 / 2 + (53 - 23) - 456 % 34);

    // equality
    printf("%d\n", 1 == 1);
    printf("%d\n", 1 == 2);
    printf("%d\n", 1 != 2 - 1);
    printf("%d\n", 0 != 2 - 1);
    printf("%d\n", 6 > 6);
    printf("%d\n", 6 >= 6);
    printf("%d\n", 7 < 6);
    printf("%d\n", 7 <= 6);
    printf("%d\n", 7 <= 7);

    return 0;
}

