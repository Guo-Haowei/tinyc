#include <stdio.h>

int main() {
    int a = 51;
    int b = 102;
    printf("%d\n", a);
    printf("%d\n", b);
    {
        int a = 10;
        printf("%d\n", a);
        {
            int b = 12213;
            printf("%d\n", b);
        }
        int c = 6789;
        printf("%d\n", b);
        printf("%d\n", c);
        {
            printf("%d\n", c);
            int b = 9 + a;
            printf("%d\n", b);
            {
                printf("%d\n", a);
                int a = b + c;
                printf("%d\n", a);
            }
        }
        printf("%d\n", b);
    }
    int c = 111 + b - a;
    printf("%d\n", a);
    printf("%d\n", c);
    return 0;
}
