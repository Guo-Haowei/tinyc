#include "cc.h"

static const char* readfile(const char* file) {
    FILE* f = fopen(file, "r");
    if (!f) {
        error("%s: No such file or directory", file);
    }

    fseek(f, 0L, SEEK_END);
    int size = ftell(f);
    fseek(f, 0L, SEEK_SET);

    char* buffer = alloc(size + 1);
    fread(buffer, size, 1, f);
    fclose(f);

    buffer[size] = '\0';
    printf("***\n%s***\n", buffer);
    return buffer;
}

struct _list_t* lex(const char* file) {
    readfile(file);
    return NULL;
}
