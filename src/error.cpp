#include "cc.hpp"

namespace cc {

static String error_line(const char* color, const String& line, int col, int len);

static String underline(const char* color, int col, int len) {
    String buf;
    buf.append(ANSI_RED);
    buf.append(color);
    for (int i = 1; i < col; ++i) {
        buf.push_back(' ');
    }
    buf.push_back('^');
    for (int i = 1; i < len; ++i) {
        buf.push_back('~');
    }
    buf.append(ANSI_RST);
    buf.push_back('\n');

    return buf;
}

static void error_interal(Level level, const char* path, int ln, int col, int len, const char* msg);

void error(Level level, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, ANSI_WHITE "%s: " ANSI_RED "error: " ANSI_RST, g_prog);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\ncompilation terminated.\n");
    va_end(args);
    exit(-1);
}

void error(Level level, const Loc& loc, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[1024];
    vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);
    error_interal(level, loc.path, loc.ln, loc.col, 1, buf);
}

String colored_line(const char* color, const String& line, int col, int len) {
    String buf;
    for (size_t i = 0; i < line.size(); ++i) {
        if (i == col - 1) {
            buf.append(color);
        } else if (i == col - 1 + len) {
            buf.append(ANSI_RST);
        }
        buf.push_back(line[i]);
    }
    buf.push_back('\n');
    return buf;
}

void error_interal(Level level, const char* path, int ln, int col, int len, const char* msg) {
    assert(ln > 0);
    const FileCache& fcache = fcache_get(path);

    const char* color = ANSI_RED;
    const char* hint = "error";
    if (level == Level::WARNING) {
        color = ANSI_MAGENTA;
        hint = "warning";
    } else if (level == Level::ERROR) {
    }

    std::cerr << format(ANSI_WHITE "%s:%d:%d %s%s:" ANSI_RST " %s\n", path, ln, col, color, hint, msg);

    // print <space>line
    const String& line = fcache.GetLines().at(ln - 1);
    std::cerr << format("  %4d | ", ln)
              << colored_line(color, line, col, len);

    // print <space>^~~~~~~~~~~~~~
    std::cerr << "       | "
              << underline(color, col, len);

    if (level == Level::FATAL || level == Level::ERROR) {
        exit(-1);
    }
}

}  // namespace cc
