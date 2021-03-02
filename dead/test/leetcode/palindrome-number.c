#include "stdio.h"

int is_palindrome(int n) {
    if (n < 0 || n > 9 && n % 10 == 0) {
        return 0;
    }

    int reversed = 0;
    while (n / 10 > reversed) {
        reversed = reversed * 10 + n % 10;
        n = n / 10;
    }

    return n == reversed || n / 10 == reversed;
}

#define TEST_CASE(n) printf("Is %d palindrone? %s.\n", n, is_palindrome(n) ? "Yes" : "No")

int main() {
    TEST_CASE(-999);
    TEST_CASE(-1);
    TEST_CASE(0);
    TEST_CASE(10);
    TEST_CASE(12);
    TEST_CASE(121);
    TEST_CASE(122);
    TEST_CASE(12344321);
    TEST_CASE(10001);
    TEST_CASE(10101);
    TEST_CASE(100001);
    TEST_CASE(344321);
    return 0;
}
