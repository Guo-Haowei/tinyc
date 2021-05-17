#include <stdio.h>

int main() {
    int i = 6;

    if (i) {
        printf("%d != 0\n", i);
    }

    if (0) printf("false\n");
    else printf("true\n");

    if (1) {
        printf("true\n");
    } else {
        printf("false\n");
    }

    if (i - 6) {
    } else if (i - i) {
    } else {
        printf("%d\n", i);
    }

    if (0) {
    }
    if (1) {
    }
    return 0;
}
