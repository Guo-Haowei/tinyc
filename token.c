#include "cc.h"

// TODO: inline
bool tkeqc(const struct Token* tk, int c) {
    cassert(tk && tk->raw);

    return tk->raw[0] == c && tk->raw[1] == '\0';
}

bool tkeqstr(const struct Token* tk, const char* str) {
    cassert(tk);
    cassert(str);

    return strcmp(tk->raw, str) == 0;
}
