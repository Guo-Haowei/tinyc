#include "toyc.h"

int main(int argc, char **argv) {
    if (argc == 1) {
        log("%s: invalid number of arguments\n", argv[0]);
        return 1;
    }

    --argc;
    ++argv;

    int verbose = false;
    if (strcmp(*argv, "-v") == 0) {
        --argc;
        ++argv;
        verbose = true;
    }

    assert(argc);

    const char *path = *argv;

    setup();

    TokenList *tokenList = tokenize(path);
    Node *root = parse(tokenList);

    validate(root);

    if (verbose) {
        log("====== print AST ======\n");
        debug_print_node(root);
    }

    // foo.c => foo.asm
    const char *p = strrchr(path, '/');
    p = p ? p + 1 : path;
    char outfile[128];
    strncpy(outfile, p, sizeof(outfile));
    int len = (int)strlen(outfile);
    assert(len + 3 < sizeof(outfile));
    outfile[len - 1] = 'a';
    outfile[len] = 's';
    outfile[len + 1] = 'm';

    FILE *fp = fopen(outfile, "w");
    if (!fp) {
        log("failed to create file %s\n", outfile);
        exit(-1);
    }
    codegen(root, fp);
    fclose(fp);

    // clean up memory
    token_list_destroy(&tokenList);
    log("====== done writing %s ======\n", *argv);
    // token leak
    // root leak

    return 0;
}
