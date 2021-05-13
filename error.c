#include "cc.h"

static int g_errors;
static int g_warnings;
const char* g_prog;

static void error_interal(int level, const char* path, int ln, int col, int len, const char* msg);

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
    vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);
    error_interal(level, loc->path, loc->ln, loc->col, 1, buf);
}

void error_tk(int level, const struct Token* tk, const char* fmt, ...) {
    cassert(tk->kind != TK_BEGIN);
    cassert(tk->kind != TK_END);
    va_list args;
    va_start(args, fmt);
    char buf[1024];
    vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);
    error_interal(level, tk->path, tk->ln, tk->col, tk->end - tk->start, buf);
}

void error_after_tk(int level, const struct Token* tk, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[1024];
    vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);
    const int col = tk->col + (tk->end - tk->start);
    error_interal(level, tk->path, tk->ln, col, 1, buf);
}

static void print_line(const char* color, const struct slice_t* slice, int col, int len) {
    for (int i = 0; i < (int)slice->len; ++i) {
        if (i == col - 1) {
            fprintf(stderr, "%s", color);
        } else if (i == col - 1 + len) {
            fprintf(stderr, "%s", ANSI_RST);
        }
        putc(slice->start[i], stderr);
    }
    putc('\n', stderr);
}

static void print_underline(const char* color, int col, int len) {
    fprintf(stderr, ANSI_RST "%s", color);
    for (int i = 1; i < col; ++i) {
        putc(' ', stderr);
    }
    putc('^', stderr);
    for (int i = 1; i < len; ++i) {
        putc('~', stderr);
    }
    fprintf(stderr, "\n" ANSI_RST);
}

void error_interal(int level, const char* path, int ln, int col, int len, const char* msg) {
    cassert(ln > 0);

    const struct FileCache* fcache = fcache_get(path);
    cassert(fcache);

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

    const struct slice_t* slice = list_at(struct slice_t*, fcache->lines, ln - 1);

    // print <space>line
    fprintf(stderr, "  %4d | ", ln);
    print_line(color, slice, col, len);

    // print <space>^~~~~~~~~~~~~~
    fprintf(stderr, "       | ");
    print_underline(color, col, len);

    if (level == LEVEL_FATAL) {
        exit(-1);
    }
}
