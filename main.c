#include "cc.h"

static void usage() {
    fprintf(stderr,
            "Usage: %s [options] file...\n"
            "Options:\n"
            "  --help   Display this information\n",
            g_prog);
}

int main(int argc, const char** argv) {
    g_prog = *argv;

    if (argc != 2) {
        usage();
        exit(-1);
    }

    init_arena();

    init_fcache();

    // init -DDEBUG=1
    init_preproc();

    struct list_t* tks = lex(argv[1]);
    check_should_exit();

    dumptks(tks);
    list_delete(tks);

    shutdown_preproc();
    shutdown_fcache();

    free_arena();
    shutdown_arena();
}
