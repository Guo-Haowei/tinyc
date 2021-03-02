#include "stdio.h"

int main() {
    int a = 101;
    int b = 102;
    printf("a is %d\n", a);
    printf("b is %d\n", b);
    {
        int a = 10;
        printf("a is %d\n", a);
        {
            int b = 12213;
            printf("b is %d\n", b);
        }
        int c = 6789;
        printf("b is %d\n", b);
    }
    int c = 111;
    printf("a is %d\n", a);
    printf("c is %d\n", c);
    return 0;
}
