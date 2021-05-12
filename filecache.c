#include "cc.h"

static const char* readfile(const char* path);
static struct map_t* g_filemap;

void init_fcache() {
    cassert(!g_filemap);
    g_filemap = map_new();

    // add built-in files
}

void shutdown_fcache() {
    cassert(g_filemap);
    for (struct list_node_t* it = g_filemap->list->front; it; it = it->next) {
        struct map_pair_t* pair = (struct map_pair_t*)(it->data);
        struct FileCache* cache = (struct FileCache*)(pair->data);
        list_delete(cache->lines);
        list_delete(cache->rawtks);
    }

    map_delete(g_filemap);
    g_filemap = NULL;
}

struct FileCache* fcache_get(const char* path) {
    cassert(g_filemap);

    // check existance
    struct map_pair_t* pair = map_find(g_filemap, path);

    if (pair) {
        return pair->data;
    }

    // read file
    const char* source = readfile(path);
    if (source == NULL) {
        return NULL;
    }

    // add cache
    // global alloc
    struct FileCache* fcache = alloc(sizeof(struct FileCache));
    fcache->source = source;
    fcache->lines = list_new();
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

        // global alloc
        struct slice_t* slice = alloc(sizeof(struct slice_t));
        slice->start = lbegin;
        slice->len = llen;
        list_push_back(fcache->lines, slice);
        lbegin = lend + 1;  // skip new line
    }

    bool succuess = map_try_insert(g_filemap, path, fcache);
    cassert(succuess);

    // raw tokens
    fcache->rawtks = lex_one(path, fcache->source);

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

    // global alloc
    char* buffer = alloc(size + 1);
    fread(buffer, size, 1, f);
    fclose(f);

    buffer[size] = '\0';

    /// TODO: remove '\r'

    return buffer;
}
