#pragma once
#include <cstdarg>
#include <iostream>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace cc {

using String = std::string;

/*
** token.cpp 
*/
enum TokenKind {
#define TOKEN(name, symbol, kw, punct) TK_##name,
#include "token.inl"
#undef TOKEN
    TK_COUNT
};

struct Token {
    TokenKind kind;          // kind of token
    const char* path;        // source path
    const char* source;      // source file
    const char* start;       // token start
    const char* end;         // token end
    const char* macroStart;  // start of the macro expanded from, if there is one
    const char* macroEnd;    // end of the macro expanded from, if there is one
    String raw;              // raw token
    int col;                 // colomn number
    int ln;                  // line number
};

const char* tk2str(const TokenKind& kind);
std::ostream& operator<<(std::ostream& o, const Token& tk);

/*
** lexer.cpp 
*/
struct Loc {
    const char* path;
    const char* source;
    const char* p;
    int ln;
    int col;
};

using TokenList = std::list<Token>;
namespace lexer {

const TokenList& lex(const char* path);
void init();

}  // namespace lexer

/*
** filecache.cpp 
*/
class FileCache {
   public:
    FileCache(const String& path);

    inline const TokenList& GetRawTokens() const { return m_rawTokens; }
    inline const std::vector<String>& GetLines() const { return m_lines; }
    inline void SetTokens(const TokenList& tokens) { m_rawTokens = tokens; }
    inline const String& GetPath() const { return m_path; }
    inline const String& GetSource() const { return m_source; }

   private:
    String m_path;
    String m_source;
    TokenList m_tokens;
    std::vector<String> m_lines;
    TokenList m_rawTokens;
};

bool fcache_has(const char* path);
bool fcache_has(const String& path);

void fcache_set_tokens(const char* path, const TokenList& tokens);
void fcache_set_tokens(const String& path, const TokenList& tokens);

FileCache& fcache_get(const char* path);
FileCache& fcache_get(const String& path);

bool file_exists(const char* path);
bool file_exists(const String& path);

using FileMap = std::unordered_map<String, FileCache>;

/*
** error.cpp 
*/

enum class Level {
    WARNING,
    ERROR,
    FATAL,
};

void error(Level level, const char* fmt, ...);
void error(Level level, const Loc& loc, const char* fmt, ...);
// void error_tk(int level, const struct Token* tk, const char* fmt, ...);
// void error_after_tk(int level, const struct Token* tk, const char* fmt, ...);

extern const char* g_prog;

// utilities

template <typename... Args>
String format(const char* fmt, Args... args) {
    int len = std::snprintf(nullptr, 0, fmt, args...);
    // DCHECK(length >= 0);
    String buf(size_t(len + 1), '\0');
    std::snprintf(buf.data(), len + 1, fmt, args...);
    return buf;
}

#define ANSI_RED "\e[1;31m"
#define ANSI_GRN "\e[1;32m"
#define ANSI_YELLOW "\e[1;33m"
#define ANSI_BLUE "\e[1;34m"
#define ANSI_MAGENTA "\e[1;35m"
#define ANSI_CYAN "\e[1;36m"
#define ANSI_WHITE "\e[1;37m"
#define ANSI_RST "\e[0m"

const char* va(const char* fmt, ...);
const char* va2(const char* fmt, ...);
void panic_impl(const char* path, const char* func, int ln, const char* msg);

#ifdef assert
#undef assert
#endif  // #ifdef assert

#define assert(cond)                                                                                 \
    if (!(cond)) {                                                                                   \
        cc::panic_impl(__FILE__, __FUNCTION__, __LINE__, cc::va("assertion failed: `(%s)`", #cond)); \
    }
#define assertfmt(cond, ...)                                                     \
    if (!(cond)) {                                                               \
        const char* msg = cc::va(__VA_ARGS__);                                   \
        cc::panic_impl(__FILE__,                                                 \
                       __FUNCTION__,                                             \
                       __LINE__,                                                 \
                       cc::va2("assertion failed: `(%s)`, \"%s\"", #cond, msg)); \
    }

#define panic(...) cc::panic_impl(__FILE__, __FUNCTION__, __LINE__, cc::va(__VA_ARGS__))

}  // namespace cc
