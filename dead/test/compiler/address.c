#include "stdio.h"

int main() {
    int a = 100;
    int* ap = &a;
    printf("a is %d\n", *ap);
    *ap = 10;
    printf("a is %d\n", *ap);
    *ap = 101;
    printf("a is %d\n", *ap);
    int p = *&a;
    printf("p is %d\n", p);
}
