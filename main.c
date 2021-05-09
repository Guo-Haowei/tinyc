#include "cc.h"

static void usage(const char* prog) {
    fprintf(stderr,
            "Usage: %s [options] file...\n"
            "Options:\n"
            "  --help   Display this information\n",
            prog);
}

int main(int argc, const char** argv) {
    if (argc != 2) {
        usage(argv[0]);
        exit(-1);
    }

    arena_init();

    lex(argv[1]);

    arena_shutdown();
}
