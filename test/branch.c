#include <stdio.h>

int main() {
    int six_6 = 6;

    if (six_6 != 0) { printf("%d != 0\n", six_6); }
    if (six_6 < 0) printf("false\n");
    else printf("true\n");
    { int div = 6; if (six_6 % div == 0) printf("%d divisible by %d\n", six_6, div); }
    { int div = 3; if (six_6 % div == 0) printf("%d divisible by %d\n", six_6, div); }
    { int div = 2; if (six_6 % div == 0) printf("%d divisible by %d\n", six_6, div); }
    { int div = 5; if (six_6 % div == 0) printf("%d divisible by %d\n", six_6, div); }

    return 0;
}
