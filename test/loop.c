#include <stdio.h>

int main() {
    int n = 6;
    while (n > 0) { printf("n: %d\n", n); n = n - 1; };
    n = 70;
    do {
        printf("n: %d\n", n);
    } while (n = n / 2);
    do printf("you should only see it once\n", n); while (0);
    return 0;
}
