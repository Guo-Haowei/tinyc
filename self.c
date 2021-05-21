// self.c
#include <stdio.h>
#include <stdlib.h>

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

    char* p = malloc(4096);
    char* pp = p;

    int c;
    while ((c = fgetc(fp)) != -1) {
        *pp = c;
        pp += 1;
    }
    *pp = 0;

    while (*p) {
        if (*p == '#' || (*p == '/' && p[1] == '/')) {
            while (*p && *p != 10) p = p + 1;
        } else if (*p == ' ' || *p == 10 || *p == 11 || *p == 13) {
            if (*p == 10) printf("\n");
            p = p + 1;
        } else {
            printf("%c", *p);
            p += 1;
        }
    }

    return 0;
}
