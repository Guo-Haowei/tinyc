#include "cc.h"

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
    fprintf(stderr, ANSI_WHITE "%s: " ANSI_RED "error: " ANSI_RST, g_prog);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\ncompilation terminated.\n");
    va_end(args);
    exit(-1);
}

static void error_interal(int level, const char* path, const char* source, int ln, int col, int len, const char* msg) {
    assert(ln > 0);

    const struct FileCache* fcache = fcache_get(path);
    assert(fcache);

    const char* color = ANSI_RED;
    const char* hint = "error";
    if (level == LEVEL_WARNING) {
        color = ANSI_MAGENTA;
        hint = "warning";
    }

    fprintf(stderr, ANSI_WHITE "%s:%d:%d %s%s:" ANSI_RST " %s\n", path, ln, col, color, hint, msg);

    const struct string_view* sv = list_at(struct string_view*, fcache->lines, ln - 1);
    putc(' ', stderr);
    int i = 0;
    for (; i < col - 1; ++i) {
        putc(sv->start[i], stderr);
    }
    fprintf(stderr, "%s", color);
    for (; i < len + col - 1; ++i) {
        putc(sv->start[i], stderr);
    }
    fprintf(stderr, ANSI_RST);
    for (; i < sv->len; ++i) {
        putc(sv->start[i], stderr);
    }
    putc('\n', stderr);

    fprintf(stderr, "%s", color);
    for (int i = 0; i < col; ++i) {
        putc(' ', stderr);
    }
    putc('^', stderr);
    for (int i = 1; i < len; ++i) {
        putc('~', stderr);
    }
    fprintf(stderr, "\n" ANSI_RST);
    if (level >= LEVEL_ERROR) {
        exit(-1);
    }
}

void error_loc(int level, const struct Loc* loc, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[1024];
    vsprintf(buf, fmt, args);
    va_end(args);
    buf[sizeof(buf) - 1] = '\0';
    error_interal(level, loc->path, loc->source, loc->ln, loc->col, 1, buf);
}

void error_tk(int level, const struct Token* tk, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[1024];
    vsprintf(buf, fmt, args);
    va_end(args);
    buf[sizeof(buf) - 1] = '\0';
    error_interal(level, tk->path, tk->source, tk->ln, tk->col, tk->end - tk->start, buf);
}
