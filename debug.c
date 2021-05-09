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
        "symbol",
        "punct",
        "int",
        "char",
        "string",
    };

    assert(kind > TOKEN_INVALID && kind < TOKEN_COUNT);

    return kNames[kind];
}

#endif // #ifdef DEBUG
