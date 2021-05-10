#include "cc.h"

static const char* readfile(const char* path);
static struct list_t* g_fcache;

void init_fcache() {
    assert(!g_fcache);
    g_fcache = _list_new();
}

void shutdown_fcache() {
    assert(g_fcache);
    for (struct list_node_t* n = g_fcache->front; n; n = n->next) {
        struct FileCache* cache = (struct FileCache*)(n->data);
        list_delete(cache->lines);
        list_delete(cache->rawtks);
    }
    list_delete(g_fcache);
}

struct FileCache* fcache_get(const char* path) {
    // check existance
    for (struct list_node_t* n = g_fcache->front; n; n = n->next) {
        struct FileCache* cache = (struct FileCache*)(n->data);
        if (strncmp(cache->path, path, sizeof(cache->path)) == 0) {
            return cache;
        }
    }

    // read file
    const char* source = readfile(path);
    if (source == NULL) {
        return NULL;
    }

    // add cache
    struct FileCache* fcache = alloc(sizeof(struct FileCache));
    fcache->source = source;
    fcache->lines = _list_new();
    strncpy(fcache->path, path, sizeof(fcache->path) - 1);

    // add lines
    int eof = 1;
    const char* lbegin = fcache->source;
    while (eof) {
        const char* lend = strchr(lbegin, '\n');

        int llen = 0;

        if (!lend) {
            llen = strlen(lbegin);
            eof = 0;
        } else {
            llen = lend - lbegin;
        }

        struct string_view* sv = alloc(sizeof(struct string_view));
        sv->start = lbegin;
        sv->len = llen;
        list_push_back(fcache->lines, sv);
        lbegin = lend + 1;  // skip new line
    }

    // raw tokens
    fcache->rawtks = lex_one(path, fcache->source);

    list_push_back(g_fcache, fcache);
    return fcache;
}

static const char* readfile(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) {
        return NULL;
    }

    fseek(f, 0L, SEEK_END);
    int size = ftell(f);
    fseek(f, 0L, SEEK_SET);

    char* buffer = alloc(size + 1);
    fread(buffer, size, 1, f);
    fclose(f);

    buffer[size] = '\0';

    /// TODO: remove '\r'

    return buffer;
}
