#include <stdio.h>

void test_literal() {
    printf("test literal\n");
    printf("%d\n", 123);
    printf("%X\n", 0xF);
    printf("%X\n", 0x1F2C);
    printf("%X\n", 0xDEADBEEF);
    printf("%X\n", 0xFFFFFFFF);
    printf("%c\n", 'a');
    printf("%c\n", '0');
    printf("%c\n", '-');
    return;
}

void test_arith() {
    printf("test arith\n");
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
    printf("%d\n", 1 + 2 + 3 + 4);
    printf("%d\n", 1 + 2 * 3 + 4);
    printf("%d\n", 1 * 2 + 3 * 4);
    printf("%d\n", 4 / 2 + 6 / 3);
    printf("%d\n", 24 / 2 / 3);
    printf("%d\n", 24 % 7);
    printf("%d\n", 24 % 3);
    printf("%d\n", 'a' + 1);
    int a = 0 - 1;
    printf("%d\n", a);
    // printf("%d\n", a + -1);
    return;
}

void test_relational() {
    printf("test ralational\n");
    printf("%d\n", 1 > 0);
    printf("%d\n", 0 < 1);
    printf("%d\n", 1 < 0);
    printf("%d\n", 0 > 1);
    printf("%d\n", 1 > 1);
    printf("%d\n", 1 < 1);
    printf("%d\n", 1 >= 0);
    printf("%d\n", 0 <= 1);
    printf("%d\n", 1 <= 0);
    printf("%d\n", 0 >= 1);
    printf("%d\n", 1 >= 1);
    printf("%d\n", 1 <= 1);
    printf("%d\n", 1 == 1);
    printf("%d\n", 1 == 2);
    printf("%d\n", 1 != 2 - 1);
    printf("%d\n", 0 != 2 - 1);
    printf("%d\n", 6 > 6);
    printf("%d\n", 6 >= 6);
    printf("%d\n", 7 < 6);
    printf("%d\n", 7 <= 6);
    printf("%d\n", 7 <= 7);
    printf("%d\n", 0xFFFF > 1);
    return;
}

void test_assign() {
    printf("test assignment\n");
    int v;
    v = 5;
    printf("%d\n", v -= 1);
    printf("%d\n", v);
    v += v + 5;
    printf("%d\n", v);
    v -= 2 - v;
    printf("%d\n", v);
    printf("%d\n", v += v * v);
    return;
}

// static void test_inc_dec() {
//     int a = 15;
//     expect(15, a++);
//     expect(16, a);
//     expect(16, a--);
//     expect(15, a);
//     expect(14, --a);
//     expect(14, a);
//     expect(15, ++a);
//     expect(15, a);
// }

// static void test_bool() {
//     expect(0, !1);
//     expect(1 ,!0);
// }

// static void test_ternary() {
//     expect(51, (1 + 2) ? 51 : 52);
//     expect(52, (1 - 1) ? 51 : 52);
//     expect(26, (1 - 1) ? 51 : 52 / 2);
//     expect(17, (1 - 0) ? 51 / 3 : 52);
//     // GNU extension
//     expect(52, 0 ?: 52);
//     expect(3, (1 + 2) ?: 52);
// }

// static void test_unary() {
//     char x = 2;
//     short y = 2;
//     int z = 2;
//     expect(-2, -x);
//     expect(-2, -y);
//     expect(-2, -z);
// }

int main() {
    test_literal();
    test_arith();
    test_relational();
    test_assign();
    return 0;
}
