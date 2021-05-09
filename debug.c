#include "cc.h"

#ifdef DEBUG

void _assert_internal(int line, const char* file, const char* assertion) {
    fprintf(stderr,
            "assertion (%s) \e[0;31mfailed\e[0m\n\ton line %d in file '%s'\n",
            assertion,
            line,
            file);
    exit(-1);
}

const char* tk2str(int kind) {
    static const char* kNames[TOKEN_COUNT] = {
        NULL,
        "Symbol",
        "Punct",
        "Integer",
        "Char",
        "String",
    };

    assert(kind > TOKEN_INVALID && kind < TOKEN_COUNT);

    return kNames[kind];
}

void dumptks(struct _list_t* tks) {
    for (struct _list_node_t* n = tks->front; n; n = n->next) {
        struct Token* tk = (struct Token*)(n->data);
        int len = tk->end - tk->start;
        assert(len > 0);
        fprintf(stderr,
                "[%7s] [f: %s, ln: %2d, col: %2d]",
                tk2str(tk->kind),
                tk->path,
                tk->ln,
                tk->col);
        fprintf(stderr, " [\e[1;31m%.*s\e[0m]\n", len, tk->start);
    }
}

#endif  // #ifdef DEBUG
