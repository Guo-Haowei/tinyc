#include "cc.h"

void _assert_internal(int line, const char* file, const char* assertion) {
    debugln(
        "assertion (%s) " ANSI_RED "failed" ANSI_RST "\n\ton line %d, in file \"%s\"",
        assertion,
        line,
        file);
    exit(-1);
}

void _panic(int line, const char* file, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr,
            "****************************************"
            "****************************************"
            "\n" ANSI_RED "[panic]" ANSI_RST "\n\t\"");
    vfprintf(stderr, fmt, args);
    debugln("\" on line %d, in file \"%s\"", line, file);
    va_end(args);
    exit(-1);
}

const char* tk2str(int kind) {
    cassert(kind >= 0 && kind < TK_COUNT);

    static const char* kNames[TK_COUNT] = {
#define TOKEN(name, symbol, kw, punct) #name,
#include "token.inl"
#undef TOKEN
    };

    return kNames[kind];
}

void dumptks(const struct list_t* tks) {
    debugln("****** Dump tokens ******");
    for (struct list_node_t* n = tks->front; n; n = n->next) {
        struct Token* tk = (struct Token*)(n->data);
        int len = tk->end - tk->start;
        cassert(len > 0);
        fprintf(stderr,
                "[%-9s] [f: " ANSI_GRN "\"%s\"" ANSI_RST ", ln: %2d, col: %2d]",
                tk2str(tk->kind),
                tk->path,
                tk->ln,
                tk->col);
        debugln(" [\e[1;31m%s\e[0m]", tk->raw);
    }
}
