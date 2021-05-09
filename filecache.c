#include "cc.h"

struct FileCache {
    char path[256];
    const char* source;
    /// TODO: use array instead
    struct _list_t* lines;
};

static struct _list_t* g_fcache;

static const char* readfile(const char* path);
static struct FileCache* fcache_get_or_add(const char* path);

void init_fcache() {
    assert(!g_fcache);
    g_fcache = _list_new();
}

void shutdown_fcache() {
    assert(g_fcache);
    for (struct _list_node_t* n = g_fcache->front; n; n = n->next) {
        struct FileCache* cache = (struct FileCache*)(n->data);
        list_delete(struct string_view*, cache->lines);
    }
    list_delete(struct FileCache*, g_fcache);
}

const char* fcache_get(const char* path) {
    struct FileCache* fcache = fcache_get_or_add(path);
    if (!fcache) {
        return NULL;
    }

    return fcache->source;
}

const struct string_view* fcache_getline(const char* path, int ln) {
    struct FileCache* fcache = fcache_get_or_add(path);
    if (!fcache) {
        return NULL;
    }

    return list_at(struct string_view*, fcache->lines, ln);
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

static struct FileCache* fcache_get_or_add(const char* path) {
    // check existance
    for (struct _list_node_t* n = g_fcache->front; n; n = n->next) {
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
        list_push_back(struct string_view*, fcache->lines, sv);
        lbegin = lend + 1;  // skip new line
    }

    list_push_back(struct FileCache*, g_fcache, fcache);
    return fcache;
}
