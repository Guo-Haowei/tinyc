#include "cc.hpp"
using namespace cc;

namespace cc {

const char* g_prog;

}  // namespace cc

static void usage() {
    std::cerr << "Usage: " << g_prog << " [options] file...\n";
    std::cerr << "Options:\n"
                 "  --help   Display this information\n";
}

int main(int argc, const char** argv) {
    g_prog = argv[0];

    if (argc != 2) {
        usage();
        exit(-1);
    }

    lexer::init();
    const TokenList& tokens = lexer::lex(argv[1]);

    for (const Token& token : tokens) {
        std::cerr << token << std::endl;
    }

    return 0;
}
