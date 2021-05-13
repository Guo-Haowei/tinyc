#include "cc.h"

static void usage() {
    fprintf(stderr,
            "Usage: %s [options] file...\n"
            "Options:\n"
            "  --help   Display this information\n",
            g_prog);
}

/// TODO: extract exe path
///   handle command line arguments
int main(int argc, const char** argv) {
    g_prog = *argv;

    if (argc != 2) {
        usage();
        exit(-1);
    }

    init_arena();
    init_global();
    init_fcache();

    // for each file
    {
        // lexer
        debugln("****** lexing ******");
        struct list_t* tks = lex(argv[1]);
        dumptks(tks);
        check_should_exit();

        // parser
        debugln("****** parsing ******");
        parse(tks);
        check_should_exit();

        // validator
        debugln("****** validaing ******");

        list_delete(tks);
        reset_tmp_arena();
    }

    shutdown_fcache();
    shutdown_global();
    shutdown_arena();
}
