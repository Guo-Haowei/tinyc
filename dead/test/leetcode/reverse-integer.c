#include "stdio.h"

int reverse(int n) {
    int neg = 0;
    if (n < 0) {
        n = -n;
        neg = 1;
    }

    int ret = 0;
    for (; n; ret = ret * 10 + n % 10, n = n / 10)
        ;
    return neg ? -ret : ret;
}

#define TEST_CASE(n) printf("%d => %d\n", n, reverse(n))

int main() {
    TEST_CASE(0);
    TEST_CASE(1);
    TEST_CASE(-1);
    TEST_CASE(123);
    TEST_CASE(123009);
    TEST_CASE(78030);
    TEST_CASE(-56789);
    return 0;
}
