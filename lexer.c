#include "cc.h"

static struct _list_t* g_locs;

static const char* getp() {
    assert(g_locs);
    return list_back(struct Loc*, g_locs)->p;
}

static int peek() {
    assert(g_locs);
    struct Loc* loc = list_back(struct Loc*, g_locs);
    return *(loc->p);
}

static int read() {
    assert(g_locs);
    struct Loc* loc = list_back(struct Loc*, g_locs);
    int c = *(loc->p);
    if (c == '\n') {
        ++loc->ln;
        loc->col = 1;
    } else {
        ++loc->col;
    }

    ++loc->p;
    return c;
}

static void skipline() {
    assert(g_locs);
    struct Loc* loc = list_back(struct Loc*, g_locs);
    while (*loc->p) {
        int c = read();
        if (c == '\n') {
            break;
        }
    }

    return;
}

static struct Loc* push_source(const char* path, const char* source) {
    struct Loc* loc = alloc(sizeof(struct Loc));
    loc->path = path;
    loc->source = source;
    loc->p = source;
    loc->ln = 1;
    loc->col = 1;
    list_push_back(struct Loc*, g_locs, loc);
    return loc;
}

static inline int is_symbol(const int c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

static inline int is_dec(const int c) {
    return '0' <= c && c <= '9';
}

static void pop_source() {
    list_pop_back(struct Loc*, g_locs);
    return;
}

void init_lexer() {
    if (g_locs) {
        list_delete(struct Loc*, g_locs);
    }

    g_locs = _list_new();
    return;
}

static struct Token* token_new(int kind) {
    assert(kind > TOKEN_INVALID && kind < TOKEN_COUNT);
    struct Loc* loc = list_back(struct Loc*, g_locs);
    struct Token* tk = alloc(sizeof(struct Token));
    tk->path = loc->path;
    tk->source = loc->source;
    tk->start = loc->p;
    tk->end = loc->p;
    tk->macroStart = NULL;
    tk->macroEnd = NULL;
    tk->extra = NULL;
    tk->col = loc->col;
    tk->ln = loc->ln;
    tk->kind = kind;
    return tk;
}

static void add_dec(struct _list_t* list) {
    assert(is_dec(peek()));
    struct Token* tk = token_new(TOKEN_INT);
    while (is_dec(peek())) {
        read();
        ++tk->end;
    }

    list_push_back(struct Token*, list, tk);
    return;
}

static void add_symbol(struct _list_t* list) {
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

    list_push_back(struct Token*, list, tk);
    return;
}

static void add_string(struct _list_t* list) {
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

    list_push_back(struct Token*, list, tk);
    return;
}

static void add_punct(struct _list_t* list) {
    struct Token* tk = token_new(TOKEN_PUNCT);

    // only one line punct
    read();
    ++tk->end;

    list_push_back(struct Token*, list, tk);
    return;
}

struct _list_t* lex(const char* path) {
    const char* source = fcache_get(path);

    if (!source) {
        error("%s: No such file or directory", path);
    }

    push_source(path, source);
    list_new(struct Token*, tks);

    int c;
    while ((c = peek())) {
        // one line comment
        if (strncmp(getp(), "//", 2) == 0) {
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
        if (strchr("#(){}=;", c)) {
            add_punct(tks);
            continue;
        }

        struct Loc* loc = list_back(struct Loc*, g_locs);
        error_loc(loc, "stray '%c' in program", c);
    }

    pop_source();

    dumptks(tks);

    return tks;
}
