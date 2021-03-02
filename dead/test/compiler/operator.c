#include "stdio.h"

#define TEST_CASE(expr) printf("%s = %d\n", #expr, expr)

int test_arith() {
    TEST_CASE(100 + 20);
    TEST_CASE(1 - 2 - 3 - 4);
    TEST_CASE(1 - (2 - (3 - 4)));
    TEST_CASE(2147483647 + 2147483647);
    TEST_CASE(100 - 4 * 3);
    TEST_CASE((100 - 5) / 3);
    TEST_CASE((5 - 100) / 7);
    TEST_CASE(16 / 8 / 4 / 2 / 1);
    TEST_CASE(58 * (1 - 16) + 8 * 4 - 18 / 5 + (123 - 423));
    printf("(100 - 5) %% 3 = %d\n", (100 - 5) % 3);
    printf("(5 - 100) %% 7 = %d\n", (5 - 100) % 7);
    return 0;
}

int test_relational() {
    TEST_CASE(4 > 2);
    TEST_CASE(4 < 2);
    TEST_CASE(4 <= 1 + 4);
    TEST_CASE(4 <= 4);
    TEST_CASE(8 == 8);
    TEST_CASE(8 != 4);
    TEST_CASE(4 == 4 > 0);
    TEST_CASE(1 && 0);
    TEST_CASE(1 && 7);
    TEST_CASE(0 || 9);
    TEST_CASE(100 && 0 || 10 && 4);
    TEST_CASE(2 == 2 || 0);
    TEST_CASE(!2);
    TEST_CASE(!!2);
    TEST_CASE(!!0);
}

int main() {
    test_arith();
    test_relational();
    return 0;
}
