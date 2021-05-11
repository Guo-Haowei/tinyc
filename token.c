#include "cc.h"

bool tkeqc(const struct Token* tk, int c) {
    cassert(tk);
    cassert(tk->end > tk->start);

    if (tk->extra) {
        panic("implement!");
    }

    const int len = tk->end - tk->start;
    return (len == 1) && (*(tk->start) == c);
}

bool tkeqstr(const struct Token* tk, const char* str) {
    cassert(tk);
    cassert(str);

    if (tk->extra) {
        panic("implement!");
    }

    const size_t len = tk->end - tk->start;
    return (len == strlen(str)) && (strncmp(str, tk->start, len) == 0);
}
