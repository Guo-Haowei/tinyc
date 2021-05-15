#include "cc.hpp"

namespace cc {

class Lexer {
   public:
    const TokenList& Lex(const char* path);

    static String s_validPunct;
    static std::vector<String> s_punctVec;
    static std::unordered_map<String, TokenKind> s_puncts;
    static std::unordered_map<String, TokenKind> s_keywords;

   private:
    char peek();
    char read();
    void shift(int n);
    void skipline();
    void setLoc(const char* path, const char* source);

    void addSymbol();
    void addDec();
    void addPunct();
    void addString();

    void addToken(TokenKind kind, int col, const char* start, const char* end);

    Loc m_loc;
    TokenList m_tokens;
};

String Lexer::s_validPunct;
std::vector<String> Lexer::s_punctVec;
std::unordered_map<String, TokenKind> Lexer::s_puncts;
std::unordered_map<String, TokenKind> Lexer::s_keywords;

static inline int is_symbol(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

static inline int is_dec(char c) {
    return '0' <= c && c <= '9';
}

const TokenList& Lexer::Lex(const char* path) {
    if (fcache_has(path)) {
        FileCache& fcache = fcache_get(path);
        return fcache.GetRawTokens();
    }

    FileCache& fcache = fcache_get(path);
    setLoc(fcache.GetPath().c_str(), fcache.GetSource().c_str());

    char c;
    while ((c = peek())) {
        if (strncmp(m_loc.p, "//", 2) == 0) {
            skipline();
            continue;
        }

        if (strncmp(m_loc.p, "/*", 2) == 0) {
            shift(2);
            for (;;) {
                if (m_loc.p[0] == '\0') {
                    panic("unexpected EOF");
                }
                if (strncmp(m_loc.p, "*/", 2) == 0) {
                    shift(2);
                    break;
                }
                read();
            }

            continue;
        }

        constexpr char kWhiteSpace[] = " \r\n\t";
        if (strchr(kWhiteSpace, c)) {
            read();
            continue;
        }

        if (is_symbol(c)) {
            addSymbol();
            continue;
        }

        // integer literal
        if (is_dec(c)) {
            addDec();
            continue;
        }

        // string literal
        if (c == '"') {
            addString();
            continue;
        }

        // // char literal
        // if (c == '\'') {
        //     add_char(tks);
        //     continue;
        // }

        if (strchr(s_validPunct.c_str(), c)) {
            addPunct();
            continue;
        }

        error(Level::FATAL, m_loc, "stray '%c' in program", c);
        read();
    }

    fcache.SetTokens(m_tokens);
    return fcache.GetRawTokens();
}

char Lexer::peek() {
    return m_loc.p[0];
}

char Lexer::read() {
    const char c = m_loc.p[0];
    if (c == '\n') {
        ++m_loc.ln;
        m_loc.col = 1;
    } else if (c == '\r') {
        panic("TODO: handle '\\r'");
    } else {
        ++m_loc.col;
    }

    ++m_loc.p;
    return c;
}

void Lexer::shift(int n) {
    while (n-- > 0) {
        read();
    }
}

void Lexer::skipline() {
    while (m_loc.p[0]) {
        const char c = read();
        if (c == '\n') {
            break;
        }
    }

    return;
}

void Lexer::setLoc(const char* path, const char* source) {
    m_loc.path = path;
    m_loc.source = source;
    m_loc.p = source;
    m_loc.ln = 1;
    m_loc.col = 1;
}

void Lexer::addToken(TokenKind kind, int col, const char* start, const char* end) {
    Token tk;
    tk.kind = kind;
    tk.path = m_loc.path;
    tk.source = m_loc.source;
    tk.start = start;
    tk.end = end;
    tk.macroStart = nullptr;
    tk.macroEnd = nullptr;
    tk.col = col;
    tk.ln = m_loc.ln;  // assume token always on the same line
    tk.raw = String(start, end - start);
    m_tokens.push_back(tk);
}

void Lexer::addSymbol() {
    assert(is_symbol(peek()));

    const char* start = m_loc.p;
    const char* end = m_loc.p;
    const int col = m_loc.col;
    for (;;) {
        read();
        ++end;
        char c = peek();
        if (!is_symbol(c) && !is_dec(c)) {
            break;
        }
    }

    addToken(TK_SYMBOL, col, start, end);
    Token& tk = m_tokens.back();
    const auto it = s_keywords.find(tk.raw);
    if (it != s_keywords.end()) {
        tk.kind = it->second;
    }
}

void Lexer::addDec() {
    assert(is_dec(peek()));
    const char* start = m_loc.p;
    const char* end = m_loc.p;
    const int col = m_loc.col;

    while (is_dec(peek())) {
        read();
        ++end;
    }

    addToken(TK_CINT, col, start, end);
}

void Lexer::addPunct() {
    for (const String& punct : s_punctVec) {
        size_t len = punct.size();
        if (strncmp(punct.c_str(), m_loc.p, len) == 0) {
            const auto it = s_puncts.find(punct);
            assert(it != s_puncts.end());
            addToken(it->second, m_loc.col, m_loc.p, m_loc.p + len);
            shift(len);
        }
    }

    return;
}

void Lexer::addString() {
    assert(peek() == '"');

    const char* start = m_loc.p;
    const char* end = m_loc.p;
    const int col = m_loc.col;
    for (;;) {
        read();
        ++end;
        // TODO: handle escape sequence
        const char c = *(end);
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

    addToken(TK_CSTR, col, start, end);
    return;
}

const TokenList& lexer::lex(const char* path) {
    if (!file_exists(path)) {
        error(Level::FATAL, "no such file or directory: '%s'", path);
    }
    Lexer lexer;
    return lexer.Lex(path);
}

void lexer::init() {
    // keywords
#define TOKEN(name, symbol, kw, punct)         \
    if (kw) {                                  \
        Lexer::s_keywords[symbol] = TK_##name; \
    }
#include "token.inl"
#undef TOKEN

    // puncts
#define TOKEN(name, symbol, kw, punct)       \
    if (punct) {                             \
        Lexer::s_puncts[symbol] = TK_##name; \
    }
#include "token.inl"
#undef TOKEN

#define TOKEN(name, symbol, kw, punct)            \
    if (punct == 1) {                             \
        Lexer::s_validPunct.push_back(symbol[0]); \
    }
#include "token.inl"
#undef TOKEN

#define TOKEN(name, symbol, kw, punct)       \
    if (punct > 0) {                         \
        Lexer::s_punctVec.push_back(symbol); \
    }
#include "token.inl"
#undef TOKEN
}

}  // namespace cc
