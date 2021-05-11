#include "cc.h"

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

// sstream
void sstream_reserve(struct sstream* ss, int cap) {
    cassert(ss);
    cassert(cap > 0);

    sstream_clear(ss);
    char* p = malloc(cap);
    cassert(p);

    ss->head = p;
    ss->tail = ss->head;
    ss->cap = cap;
    return;
}

void sstream_clear(struct sstream* ss) {
    if (ss->head) {
        free(ss->head);
        ss->head = NULL;
    }

    ss->tail = NULL;
    ss->cap = 0;
    return;
}

const char* sstream_get(struct sstream* ss) {
    return ss->head;
}

void sstream_append(struct sstream* ss, const char* str) {
    cassert(ss && ss->head);

    const size_t len = strlen(str);
    cassert(ss->head + ss->cap > ss->tail + len);
    strcpy(ss->tail, str);
    ss->tail += len;
}

void sstream_appendfmt(struct sstream* ss, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[1024];
    vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);
    sstream_append(ss, buf);
}

void sstream_push_back(struct sstream* ss, const char c) {
    cassert(ss->head + ss->cap > ss->tail + 1);
    ss->tail[0] = c;
    ss->tail[1] = '\0';
    ++ss->tail;
}

void shortenpath(char* inpath, size_t maxsize) {
    char pathcpy[4096];
    char* pathlist[128];
    int pathlistlen = 0;

    cassert(maxsize < sizeof(pathcpy));

    strncpy(pathcpy, inpath, maxsize);

    for (char* p = pathcpy; p; ++pathlistlen) {
        pathlist[pathlistlen] = p;
        p = strchr(p, '/');
        if (p) {
            *p = '\0';
            ++p;
        }
    }

    list_new(dirs);

    for (int i = 0; i < pathlistlen; ++i) {
        const char* dir = pathlist[i];
        if (*dir == '\0') {
            continue;
        }

        if (strcmp(dir, ".") == 0) {
            continue;
        }

        if (strcmp(dir, "..") == 0) {
            if (!list_empty(dirs)) {
                list_pop_back(const char*, dirs);
                continue;
            }
        }

        list_push_back(dirs, dir);
    }

    cassert(list_len(dirs));

    make_sstream(ss, maxsize);
    for (struct list_node_t* it = dirs->front; it; it = it->next) {
        sstream_push_back(&ss, '/');
        const char* dir = it->data;
        sstream_append(&ss, dir);
    }

    memset(inpath, 0, maxsize);
    strncpy(inpath, ss.head + 1, maxsize);
    sstream_clear(&ss);
    list_delete(dirs);
}

const char* path_concat(const char* basefile, const struct slice_t* path) {
    static char s_buffer[MAX_PATH_LEN];
    strncpy(s_buffer, basefile, sizeof(s_buffer) - 1);

    char* p = s_buffer;
    char* last_slash = strrchr(s_buffer, '/');
    if (last_slash) {
        p = last_slash + 1;
    }
    *p = '\0';

    /// TODO: string builder
    const int cpylen = sizeof(s_buffer) - (s_buffer - p);
    strncpy(p, path->start, cpylen);

    shortenpath(s_buffer, sizeof(s_buffer));

    return s_buffer;
}

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
