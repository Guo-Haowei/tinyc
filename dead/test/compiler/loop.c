#include "stdio.h"

int main() {
    {
        int n = 7;
        while (--n > 0) {
            printf("%d,", n);
        }
        printf("\n");
    }

    int it = 7;
    while (it-- > 0) {
        printf("%d, ", it);
    }
    printf("\n");

    for (int it = 8; it; it = it - 2) {
        printf("%d, ", it);
    }
    printf("\n");
    int size = 11;
    for (int i = 0; i < size; i = i + 1) {
        for (int j = 0; j < size; j = j + 1) {
            if ((i + j) % 2 == 0)
                printf("$$");
            else
                printf("  ");
        }
        printf("\n");
    }
    for (;;) {
        printf("You should only see this once\n");
        break;
    }
    while (0) {
        printf("You shouldn't see this\n");
    }
    {
        int sum = 0;
        for (int i = 1; i <= 10; i++)
            sum = sum + i;
        printf("sum is %d\n", sum);
    }
    {
        int sum = 0;
        int i = 0;
        while (1) {
            sum = sum + i++;
            if (i > 10)
                break;
            else
                continue;
        }
        printf("i is %d, sum is %d\n", i, sum);
    }
    {
        int sum = 0;
        int i = 11;
        while (i-- > 0)
            sum = sum + i;
        printf("i is %d, sum is %d\n", i, sum);
    }
    {
        int sum = 0;
        int i = 0;
        for (; i <= 10;) {
            sum = sum + i++;
        }
        printf("i is %d, sum is %d\n", i, sum);
    }
    {
        int sum = 0;
        for (int i = 0; i < 10; sum = sum + ++i)
            ;
        printf("sum is %d\n", sum);
    }
    {
        int a = 0;
        for (; a < 3; a = a + 1)
            a = a * 2;
        printf("a is %d\n", a);
    }
    {
        int ans = 0;
        for (int i = 0; i < 10; i = i + 1)
            for (int j = 0; j < 10; j = j + 1)
                if ((i / 2) * 2 == i)
                    break;
                else
                    ans = ans + i;
        printf("ans is %d\n", ans);
    }
    {
        int a = 0;
        while (a < 5)
            a = a + 2;
        printf("a is %d\n", a);
    }
    {
        int a = 0;
        int b = 1;

        while (a < 5) {
            a = a + 2;
            b = b * a;
        }
        printf("a is %d, b is %d\n", a, b);
    }
    {
        int i = 10;
        int j = 5;
        for (int i = 11; --j; ++i) {
            int i = -9;
            printf("i is %d, j is %d\n", i, j);
        }
    }
    {
        int a = 1;
        while (a / 3 < 20) {
            int b = 1;
            while (b < 10)
                b = b * 2;
            a = a + b;
        }
        printf("a is %d\n", a);
    }
    {
        int i = 0;
        int j = 0;
        for (; i < 10; ++i) {
            int k = i;
            for (int i = k; i < 10; ++i)
                j = j + 1;
        }
        printf("i is %d, j is %d\n", i, j);
    }
    {
        int i = 0;
        int j = 0;
        for (int i = 100; i > 0; --i) {
            int i = 0;
            int j = j * 2 + i;
        }
        printf("i is %d, j is %d\n", i, j);
    }
    {
        int sum = 0;
        for (int i = 0; i < 10; i = i + 1) {
            if ((sum / 2) * 2 != sum)
                continue;
            sum = sum + i;
        }
        printf("sum is %d\n", sum);
    }
    return 0;
}
