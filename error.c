#include "cc.h"

#define COLOR_ERR "\e[1;31m"
#define RESET "\e[0m"

void panic(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "\n[panic]\n\t");
    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);
    va_end(args);
    exit(-1);
}

void error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, COLOR_ERR "error: " RESET);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\ncompilation terminated.\n");
    va_end(args);
    exit(-1);
}
