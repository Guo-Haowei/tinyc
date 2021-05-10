#include "cc.h"

struct Loc g_loc;

static inline int peek() {
    return *(g_loc.p);
}

static int read() {
    const int c = *(g_loc.p);
    if (c == '\n') {
        ++g_loc.ln;
        g_loc.col = 1;
    } else {
        ++g_loc.col;
    }

    ++g_loc.p;
    return c;
}

static void skipline() {
    while (*g_loc.p) {
        int c = read();
        if (c == '\n') {
            break;
        }
    }

    return;
}

static void loc_reset(const char* path, const char* source) {
    g_loc.path = path;
    g_loc.source = source;
    g_loc.p = source;
    g_loc.ln = 1;
    g_loc.col = 1;
    return;
}

static inline int is_symbol(const int c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

static inline int is_dec(const int c) {
    return '0' <= c && c <= '9';
}

static struct Token* token_new(int kind) {
    assert(kind > TOKEN_INVALID && kind < TOKEN_COUNT);
    struct Token* tk = alloc(sizeof(struct Token));
    tk->path = g_loc.path;
    tk->source = g_loc.source;
    tk->start = g_loc.p;
    tk->end = g_loc.p;
    tk->macroStart = NULL;
    tk->macroEnd = NULL;
    tk->extra = NULL;
    tk->col = g_loc.col;
    tk->ln = g_loc.ln;
    tk->kind = kind;
    return tk;
}

static void add_dec(struct list_t* tks) {
    assert(is_dec(peek()));
    struct Token* tk = token_new(TOKEN_INT);
    while (is_dec(peek())) {
        read();
        ++tk->end;
    }

    list_push_back(tks, tk);
    return;
}

static void add_symbol(struct list_t* tks) {
    assert(is_symbol(peek()));
    struct Token* tk = token_new(TOKEN_SYMBOL);
    for (;;) {
        read();
        ++tk->end;
        int c = peek();
        if (!is_symbol(c) && !is_dec(c)) {
            break;
        }
    }

    list_push_back(tks, tk);
    return;
}

static void add_string(struct list_t* tks) {
    assert(peek() == '"');
    struct Token* tk = token_new(TOKEN_STRING);
    for (;;) {
        read();
        ++tk->end;
        /// TODO: handle escape sequence
        const int c = *(tk->end);
        if (c == '"') {
            if (*(tk->end - 1) != '\\') {
                read();
                ++tk->end;
                break;
            }
        }

        if (c == '\0') {
            break;
        }
    }

    list_push_back(tks, tk);
    return;
}

static void add_punct(struct list_t* tks) {
    struct Token* tk = token_new(TOKEN_PUNCT);

    // only one line punct
    read();
    ++tk->end;

    list_push_back(tks, tk);
    return;
}

struct list_t* lex_one(const char* path, const char* source) {
    list_new(tks);
    loc_reset(path, source);

    int c;
    while ((c = peek())) {
        // one line comment
        if (strncmp(g_loc.p, "//", 2) == 0) {
            skipline();
            continue;
        }

        // whitespace
        if (strchr(" \n\r\t", c)) {
            read();
            continue;
        }

        // integer literal
        if (is_dec(c)) {
            add_dec(tks);
            continue;
        }

        // symbol
        if (is_symbol(c)) {
            add_symbol(tks);
            continue;
        }

        // string literal
        if (c == '"') {
            add_string(tks);
            continue;
        }

        // punct
        if (strchr("#(){}=,;", c)) {
            add_punct(tks);
            continue;
        }

        error_loc(LEVEL_ERROR, &g_loc, "stray '%c' in program", c);
    }

    return tks;
}

struct list_t* lex(const char* path) {
    struct FileCache* fcache = fcache_get(path);

    if (!fcache) {
        error("%s: No such file or directory", path);
    }

    // preprocess
    struct list_t* tks = preproc(fcache->rawtks);

    return tks;
}
