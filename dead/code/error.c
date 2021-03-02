#include "toyc.h"

void log_error_at_token(char const *msg, Token const *token) {
    log_error_at_loc(msg, &token->loc, token->len);
}

void log_error_at_loc(char const *msg, Loc const *loc, int span) {
    assert(msg);
    assert(loc);
    assert(loc->ln > 0);
    assert(loc->col > 0);

    log("%s:%d:%d: error: %s\n",
        loc->fileCache->path,
        loc->ln,
        loc->col,
        // color,
        msg);
    log("%5d | %s\n", loc->ln, find_line(loc));
    log("      |%.*s%.*s\n",
        loc->col,
        "                                                                                ",
        span,
        "^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    exit(-1);
}

void log_unexpected_token(Token *const token) {
    assert(token);

    log_error_at_loc(
        va("unexpected token \"%.*s\"", token->len, token->loc.p),
        &token->loc,
        token->len);
}
