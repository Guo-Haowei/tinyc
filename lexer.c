#include "cc.h"

static struct Loc g_loc;
static struct map_t* g_puncts;
static struct map_t* g_keywords;
static char g_validpuncts[128];

static int peek() {
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

static void shift(int n) {
    while (n-- > 0) {
        read();
    }
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

static struct Token* token_new(int kind, int col, const char* start, const char* end) {
    cassert(kind > TK_INVALID && kind < TK_COUNT);
    struct Token* tk = allocg(sizeof(struct Token));
    tk->path = g_loc.path;
    tk->source = g_loc.source;
    tk->start = start;
    tk->end = end;
    tk->macroStart = NULL;
    tk->macroEnd = NULL;
    tk->col = col;
    tk->ln = g_loc.ln;  // assume token always on the same line
    tk->kind = kind;

    size_t len = tk->end - tk->start;
    tk->raw = allocg(len + 1);
    strncpy(tk->raw, tk->start, len);
    tk->raw[len] = '\0';
    return tk;
}

static void add_dec(struct list_t* tks) {
    cassert(is_dec(peek()));

    const char* start = g_loc.p;
    const char* end = g_loc.p;
    int col = g_loc.col;
    while (is_dec(peek())) {
        read();
        ++end;
    }

    struct Token* tk = token_new(TK_CINT, col, start, end);
    list_push_back(tks, tk);
    return;
}

static void add_symbol(struct list_t* tks) {
    cassert(is_symbol(peek()));

    const char* start = g_loc.p;
    const char* end = g_loc.p;
    int col = g_loc.col;
    for (;;) {
        read();
        ++end;
        int c = peek();
        if (!is_symbol(c) && !is_dec(c)) {
            break;
        }
    }

    struct Token* tk = token_new(TK_SYMBOL, col, start, end);

    // check if keyword
    struct map_pair_t* pair = map_find(g_keywords, tk->raw);
    if (pair) {
        tk->kind = (int)pair->data;
    }

    list_push_back(tks, tk);
    return;
}

static void add_string(struct list_t* tks) {
    cassert(peek() == '"');

    const char* start = g_loc.p;
    const char* end = g_loc.p;
    int col = g_loc.col;
    for (;;) {
        read();
        ++end;
        /// TODO: handle escape sequence
        const int c = *(end);
        if (c == '"') {
            if (*(end - 1) != '\\') {
                read();
                ++end;
                break;
            }
        }

        if (c == '\0') {
            break;
        }
    }

    struct Token* tk = token_new(TK_CSTR, col, start, end);
    list_push_back(tks, tk);
    return;
}

static void add_char(struct list_t* tks) {
    cassert(peek() == '\'');

    const char* start = g_loc.p;
    const char* end = g_loc.p;
    int col = g_loc.col;
    for (;;) {
        read();
        ++end;
        /// TODO: handle escape sequence
        const int c = *(end);
        if (c == '\'') {
            if (*(end - 1) != '\\') {
                read();
                ++end;
                break;
            }
        }

        if (c == '\0') {
            break;
        }
    }

    struct Token* tk = token_new(TK_CCHAR, col, start, end);
    list_push_back(tks, tk);
    return;
}

static void add_punct(struct list_t* tks) {
    for (struct list_node_t* it = g_puncts->list->front; it; it = it->next) {
        struct map_pair_t* pair = ((struct map_pair_t*)(it->data));
        const char* punct = pair->key;
        size_t len = strlen(punct);
        if (strncmp(punct, g_loc.p, len) == 0) {
            int kind = (int)(pair->data);
            struct Token* tk = token_new(kind, g_loc.col, g_loc.p, g_loc.p + len);
            list_push_back(tks, tk);
            shift(len);
            return;
        }
    }

    return;
}

struct list_t* lex_one(const char* path, const char* source) {
    struct list_t* tks = list_new();
    loc_reset(path, source);

    int c;
    while ((c = peek())) {
        // comment
        if (strncmp(g_loc.p, "//", 2) == 0) {
            skipline();
            continue;
        }

        // comment block
        if (strncmp(g_loc.p, "/*", 2) == 0) {
            shift(2);  // skip '/*'

            for (;;) {
                if (g_loc.p[0] == '\0') {
                    panic("unexpected EOF");
                }
                if (strncmp(g_loc.p, "*/", 2) == 0) {
                    shift(2);  // skip '*/'
                    break;
                }
                read();
            }

            continue;
        }

        // whitespace
        if (strchr(" \n\r\t", c)) {
            read();
            continue;
        }

        // symbol
        if (is_symbol(c)) {
            add_symbol(tks);
            continue;
        }

        // integer literal
        if (is_dec(c)) {
            add_dec(tks);
            continue;
        }

        // string literal
        if (c == '"') {
            add_string(tks);
            continue;
        }

        // char literal
        if (c == '\'') {
            add_char(tks);
            continue;
        }

        // try add punct
        if (strchr(g_validpuncts, c)) {
            add_punct(tks);
            continue;
        }

        /// TODO: stray character
        error_loc(LEVEL_FATAL, &g_loc, "stray '%c' in program", c);
        read();
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

void init_tk_global() {
    cassert(g_puncts == NULL);
    cassert(g_keywords == NULL);

    g_puncts = map_new();
#define TOKEN(name, symbol, kw, punct)                             \
    if (punct) {                                                   \
        bool result = map_try_insert(g_puncts, symbol, TK_##name); \
        cassert(result);                                           \
    }
#include "token.inl"
#undef TOKEN

    g_keywords = map_new();
#define TOKEN(name, symbol, kw, punct)                               \
    if (kw) {                                                        \
        bool result = map_try_insert(g_keywords, symbol, TK_##name); \
        cassert(result);                                             \
    }
#include "token.inl"
#undef TOKEN

    size_t idx = 0;
#define TOKEN(name, symbol, kw, punct)  \
    if (punct == 1) {                   \
        g_validpuncts[idx] = symbol[0]; \
        ++idx;                          \
    }
#include "token.inl"
#undef TOKEN
    cassert(idx < sizeof(g_validpuncts));
}

void free_tk_global() {
    cassert(g_puncts != NULL);
    cassert(g_keywords != NULL);

    map_delete(g_puncts);
    map_delete(g_keywords);

    g_puncts = NULL;
    g_keywords = NULL;
}
