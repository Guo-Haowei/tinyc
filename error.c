#include "cc.h"

#define COLOR_WHT "\e[1;37m"
#define COLOR_RED "\e[1;31m"
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
    fprintf(stderr, COLOR_WHT "%s: " COLOR_RED "error: " RESET, g_prog);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\ncompilation terminated.\n");
    va_end(args);
    exit(-1);
}

static void error_interal(const char* path, const char* source, int ln, int col, int len, const char* err) {
    fprintf(stderr, COLOR_WHT "%s:%d:%d " COLOR_RED "error:" RESET " %s\n", path, ln, col, err);
    const struct string_view* sv = fcache_getline(path, ln - 1);
    fprintf(stderr, "%5d | %.*s\n", ln, sv->len, sv->start);
    fprintf(stderr, "      |" COLOR_RED);
    for (int i = 0; i < col; ++i) {
        putc(' ', stderr);
    }
    for (int i = 0; i < len; ++i) {
        putc('^', stderr);
    }
    fprintf(stderr, " %s \n" RESET, err);
}

void error_loc(struct Loc* loc, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[1024];
    vsprintf(buf, fmt, args);
    va_end(args);
    buf[sizeof(buf) - 1] = '\0';
    error_interal(loc->path, loc->source, loc->ln, loc->col, 1, buf);
    exit(-1);
}
