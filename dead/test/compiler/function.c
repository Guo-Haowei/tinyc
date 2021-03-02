#include "stdio.h"

int add_three(int a, int b, int c) {
    return a + b + c;
}

int fib(int n) {
    return n < 2 ? n : fib(n - 1) + fib(n - 2);
}

int main() {
    printf("1+2+3+4+5+6+7+8+9=%d\n",
           add_three(add_three(1, 2, 3), add_three(4, 5, 6), add_three(7, 8, 9)));
    printf("fib 0: %d\n", fib(0));
    printf("fib 1: %d\n", fib(1));
    printf("fib 2: %d\n", fib(2));
    printf("fib 3: %d\n", fib(3));
    printf("fib 4: %d\n", fib(4));
    printf("fib 5: %d\n", fib(5));
    return 0;
}
