#include "cc.h"

static int g_errors;
static int g_warnings;
const char* g_prog;

static void error_interal(int level, const char* path, const char* source, int ln, int col, int len, const char* msg);

void panic(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr,
            "****************************************"
            "****************************************"
            "\n[panic]\n\t");
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

void check_should_exit() {
    if (g_errors) {
        debugln("compilation terminated.");
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

void error_after_tk(int level, const struct Token* tk, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[1024];
    vsprintf(buf, fmt, args);
    va_end(args);
    buf[sizeof(buf) - 1] = '\0';
    const int col = tk->col + (tk->end - tk->start);
    error_interal(level, tk->path, tk->source, tk->ln, col, 1, buf);
}

static void print_line(const char* color, const struct string_view* sv, int col, int len) {
    putc(' ', stderr);
    for (int i = 0; i < sv->len; ++i) {
        if (i == col - 1) {
            fprintf(stderr, "%s", color);
        } else if (i == col - 1 + len) {
            fprintf(stderr, "%s", ANSI_RST);
        }
        putc(sv->start[i], stderr);
    }
    putc('\n', stderr);
}

static void print_underline(const char* color, int col, int len) {
    fprintf(stderr, ANSI_RST "%s", color);
    for (int i = 0; i < col; ++i) {
        putc(' ', stderr);
    }
    putc('^', stderr);
    for (int i = 1; i < len; ++i) {
        putc('~', stderr);
    }
    fprintf(stderr, "\n" ANSI_RST);
}

void error_interal(int level, const char* path, const char* source, int ln, int col, int len, const char* msg) {
    assert(ln > 0);

    const struct FileCache* fcache = fcache_get(path);
    assert(fcache);

    const char* color = ANSI_RED;
    const char* hint = "error";
    if (level == LEVEL_WARNING) {
        color = ANSI_MAGENTA;
        hint = "warning";
        ++g_warnings;
    } else if (level == LEVEL_ERROR) {
        ++g_errors;
    }

    fprintf(stderr, ANSI_WHITE "%s:%d:%d %s%s:" ANSI_RST " %s\n", path, ln, col, color, hint, msg);

    const struct string_view* sv = list_at(struct string_view*, fcache->lines, ln - 1);
    // debugln(" %.*s", sv->len, sv->start);

    // print <space>line
    print_line(color, sv, col, len);

    // print <space>^~~~~~~~~~~~~~
    print_underline(color, col, len);

    if (level == LEVEL_FATAL) {
        exit(-1);
    }
}
