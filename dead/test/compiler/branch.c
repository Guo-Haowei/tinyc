#include "stdio.h"

int main() {
    int a = 78 * 3 - 2 / 2;
    int b = 323 + a;
    int c = a < b ? (a = a + 1) : (b = b + 1);
    int d = 0;
    printf("a: %d, b: %d, c: %d, d: %d\n", a, b, c, d);
    d = a > b ? (a = a + 1) : (b = b + 1);
    printf("a: %d, b: %d, c: %d, d: %d\n", a, b, c, d);
    int e = a > b ? 5 : c > d ? 6
                              : 7;
    int f = a < b ? 5 : c < d ? 6
                              : 7;
    printf("e: %d, f: %d\n", e, f);
    1 ? e = 1 : (e = 0);
    0 ? f = 1 : (f = 0);
    printf("e: %d, f: %d\n", e, f);
    /// branching
    a = 0;
    if (a)
        printf("a is 0\n");
    else
        printf("a is not 0\n");
    a = 9999;
    if (a) {
        printf("a is 0\n");
    } else {
        printf("a is not 0\n");
    }
    b = 100;
    if (b < a)
        printf("b < a\n");
    e = 1;
    printf("e = %d\n", e);
    if (e > 10)
        printf("e > 10\n");
    else if (e > 5)
        printf("e > 5\n");
    else if (e > 3)
        printf("e > 3\n");
    else
        printf("e <= 3\n");
    e = 4;
    printf("e = %d\n", e);
    if (e > 10)
        printf("e > 10\n");
    else if (e > 5)
        printf("e > 5\n");
    else if (e > 3)
        printf("e > 3\n");
    else
        printf("e <= 3\n");
    e = 10;
    printf("e = %d\n", e);
    if (e > 10)
        printf("e > 10\n");
    else if (e > 5)
        printf("e > 5\n");
    else if (e > 3)
        printf("e > 3\n");
    else
        printf("e <= 3\n");
    e = 13;
    printf("e = %d\n", e);
    if (e > 10)
        printf("e > 10\n");
    else if (e > 5)
        printf("e > 5\n");
    else if (e > 3)
        printf("e > 3\n");
    else
        printf("e <= 3\n");
    return 0;

    int age = 45;
    printf("Hello, I am %s!. I am %d years old.\n", "Jimmy Scott", age);
    int numOfChildren = 1;
    if (numOfChildren < 2) {
        printf("I have %d child.\n", numOfChildren);
    } else
        printf("I have %d children.\n", numOfChildren);

    int num = 11;
    int prime = num % 2 == 0 ? 1 : num % 3 == 0 ? 1
                                                : num % 5 == 0;
    printf("%d is %sa prime number.\n", num, prime ? "" : "not ");
    num = 25;
    prime = num % 2 == 0 ? 1 : num % 3 == 0 ? 1
                                            : num % 5 == 0;
    printf("%d is %sa prime number.\n", num, prime ? "" : "not ");
    return 0;
}
