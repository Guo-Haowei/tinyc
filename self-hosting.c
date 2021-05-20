#include <stdio.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s file [args...]\n", *argv);
        return 1;
    }

    void* fp = fopen(*(argv + 1), "r");
    if (!fp) {
        printf("file '%s' does not exist\n", *(argv + 1));
        return 1;
    }

    // int i = 0, c;
    // // int i = 0, c;
    int c;
    while ((c = fgetc(fp)) != -1) {
        printf("%c", c);
    }

    return 0;
}
