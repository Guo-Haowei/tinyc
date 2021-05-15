#include "cc.hpp"

namespace cc {

const char* tk2str(const TokenKind& kind) {
    static const char* s_names[TK_COUNT] = {
#define TOKEN(name, symbol, kw, punct) #name,
#include "token.inl"
#undef TOKEN
    };

    return s_names[kind];
}

std::ostream& operator<<(std::ostream& o, const Token& tk) {
    o << format("-- %-9s ", tk2str(tk.kind));
    o << format(
        "[f: " ANSI_GRN "\"%s\"" ANSI_RST ", ln: %2d, col: %2d]",
        tk.path,
        tk.ln,
        tk.col);
    o << " [" << ANSI_RED << tk.raw << ANSI_RST << "]";
    return o;
}

}  // namespace cc
