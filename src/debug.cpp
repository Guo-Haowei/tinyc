#include "cc.hpp"

namespace cc {

static inline void error_loc(std::ostream& o, const char* path, const char* func, int line) {
    const char* p = strrchr(path, '/');
    const char* short_path = p ? p + 1 : path;
    o << ", " << short_path << ':' << func << "():" << line << std::endl;
}

const char* va(const char* fmt, ...) {
    constexpr int kBufSize = 4096;
    constexpr int kBufCnt = 8;
    static char s_buffer[kBufSize];

    char* buf = s_buffer;
    va_list args;
    va_start(args, fmt);
    int len = std::vsnprintf(buf, kBufSize - 1, fmt, args);
    va_end(args);
    return buf;
}

const char* va2(const char* fmt, ...) {
    constexpr int kBufSize = 4096;
    constexpr int kBufCnt = 8;
    static char s_buffer[kBufSize];

    char* buf = s_buffer;
    va_list args;
    va_start(args, fmt);
    int len = std::vsnprintf(buf, kBufSize - 1, fmt, args);
    va_end(args);
    return buf;
}

void panic_impl(const char* path, const char* func, int ln, const char* msg) {
    std::cerr << ANSI_RED "[panic]" ANSI_RST " at '"
              << msg << '\'';
    error_loc(std::cerr, path, func, ln);
    std::cerr << std::flush;
    exit(-1);
}

}  // namespace cc
