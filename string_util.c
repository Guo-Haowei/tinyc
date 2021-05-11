#include "cc.h"

// string_buf
void sb_reserve(struct string_buf* sb, int cap) {
    assert(sb);
    assert(cap > 0);

    sb_clear(sb);
    sb->buf = calloc(1, cap);
    if (!sb->buf) {
        panic("Failed to calloc %d bytes", cap);
    }

    sb->cap = cap;
    return;
}

void sb_clear(struct string_buf* sb) {
    if (sb->buf) {
        free(sb->buf);
        sb->buf = NULL;
    }

    sb->cap = 0;
    sb->len = 0;
    return;
}

void sb_append_str(struct string_buf* sb, const char* str) {

}

char* shortpath(char* path) {
    char pathcpy[4096];
    char* pathlist[128];
    int pathlistlen = 0;

    int pathlen = strlen(path);
    assert(pathlen < sizeof(pathcpy));

    strncpy(pathcpy, path, pathlen);
    printf("full path '%s'\n", pathcpy);

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

    /// TODO: string buf
    for (struct list_node_t* it = dirs->front; it; it = it->next) {
        printf("'%s'\n", (char*)it->data);
    }

    list_delete(dirs);

    return path;
}

const char* filepath(const char* basefile, const struct string_view* path) {
    static char s_buffer[MAX_PATH_LEN];
    strncpy(s_buffer, basefile, sizeof(s_buffer) - 1);

    // debugln("basefile is %s", s_buffer);

    char* p = s_buffer;
    char* last_slash = strrchr(s_buffer, '/');
    if (last_slash) {
        p = last_slash + 1;
    }
    *p = '\0';

    /// TODO: string builder
    const int cpylen = MIN(path->len, sizeof(s_buffer) - 1 - (s_buffer - p));
    strncpy(p, path->start, cpylen);

    return s_buffer;
}
