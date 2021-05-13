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

    static const char* s_names[TK_COUNT] = {
#define TOKEN(name, symbol, kw, punct) #name,
#include "token.inl"
#undef TOKEN
    };

    return s_names[kind];
}

const char* tk2prettystr(int kind) {
    cassert(kind >= 0 && kind < TK_COUNT);

    static const char* s_names[TK_COUNT] = {
#define TOKEN(name, symbol, kw, punct) symbol,
#include "token.inl"
#undef TOKEN
    };

    return s_names[kind];
}

void dumptks(const struct list_t* tks) {
    for (struct list_node_t* n = tks->front; n; n = n->next) {
        struct Token* tk = (struct Token*)(n->data);
        fprintf(stderr, "-- %-9s ", tk2str(tk->kind));
        if (tk->kind == TK_BEGIN || tk->kind == TK_END) {
            fputc('\n', stderr);
            continue;
        }

        fprintf(stderr,
                "[f: " ANSI_GRN "\"%s\"" ANSI_RST ", ln: %2d, col: %2d]",
                tk->path,
                tk->ln,
                tk->col);
        debugln(" [\e[1;31m%s\e[0m]", tk->raw);
    }
}

const char* dt2str(int kind) {
    cassert(kind >= 0 && kind <= DT_COUNT);
    static const char* s_names[DT_COUNT] = {
        "<error-type>",
        "int",
        "char",
        "void",
        "[]",
        "*"};

    return s_names[kind];
}

void dumpfunc(FILE* f, const struct Symbol* func) {
    cassert(f);
    cassert(func);
    cassert(func->kind == SYMBOL_FUNC);

    fprintf(f, "%s %s(", datatype_string(func->retType), func->symbol->raw);
    for (const struct list_node_t* it = func->params->front; it; it = it->next) {
        const struct Symbol* param = it->data;
        fprintf(f, "%s %s", datatype_string(param->dataType), param->symbol->raw);
        if (it->next != NULL) {
            fputc(',', f);
            fputc(' ', f);
        }
    }
    fprintf(f, ")");
}
