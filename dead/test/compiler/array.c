#include "stdio.h"

int main() {
    int a1 = 1;
    int a2 = 2;
    int arr[4] = {a1, a2, a1 + a2, 4};
    for (int i = 0; i < 4; ++i) {
        printf("%d\n", arr[i]);
    }
    for (int i = 0; i < 4; ++i) {
        arr[i] = 4 - i;
        printf("%d\n", arr[i]);
    }
    for (int i = 0; i < 4; ++i) {
        printf("%d\n", arr[i]--);
    }
    for (int i = 0; i < 4; ++i) {
        printf("%d\n", ++arr[i]);
    }
}
