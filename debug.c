#include "cc.h"

void _assert_internal(int line, const char* file, const char* assertion) {
    debugln(
        "assertion (%s) \e[0;31mfailed\e[0m\n\ton line %d in file '%s'",
        assertion,
        line,
        file);
    exit(-1);
}

const char* tk2str(int kind) {
    static const char* kNames[TOKEN_COUNT] = {
        "<error-kind>",
        "Symbol",
        "Keyword",
        "Punct",
        "Integer",
        "Char",
        "String",
    };

    cassert(kind > TOKEN_INVALID && kind < TOKEN_COUNT);

    return kNames[kind];
}

void dumptks(const struct list_t* tks) {
    debugln("****** Dump tokens ******");
    for (struct list_node_t* n = tks->front; n; n = n->next) {
        struct Token* tk = (struct Token*)(n->data);
        int len = tk->end - tk->start;
        cassert(len > 0);
        fprintf(stderr,
                "[%7s] [f: " ANSI_GRN "\"%s\"" ANSI_RST ", ln: %2d, col: %2d]",
                tk2str(tk->kind),
                tk->path,
                tk->ln,
                tk->col);
        debugln(" [\e[1;31m%.*s\e[0m]", len, tk->start);
    }
}
