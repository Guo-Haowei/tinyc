#include "cc.h"

static struct list_t* get_preproc_stmt(struct list_node_t** it, int ln) {
    struct list_t* stmt = list_new();

    while (*it) {
        const struct Token* tk = (*it)->data;
        if (streq(tk->raw, "\\")) {
            panic("todo");
        }

        cassert(tk->ln >= ln);
        if (tk->ln != ln) {
            break;
        }

        list_push_back(stmt, tk);

        *it = (*it)->next;
    }

    return stmt;
}

struct list_t* preproc(struct list_t* tks) {
    struct list_t* ntks = list_new();

    struct list_node_t* it = tks->front;
    while (it) {
        const struct Token* tk = it->data;

        /// TODO: refactor this
        bool is_preproc = false;
        if (tk->raw[0] == '#' && tk->raw[1] == '\0') {
            is_preproc = true;
            if (it->prev) {
                const struct Token* ptk = it->prev->data;
                if (tk->ln == ptk->ln) {
                    is_preproc = false;
                }
            }
        }

        if (!is_preproc) {
            list_push_back(ntks, tk);

            int len = tk->end - tk->start;
            cassert(len > 0);
            it = it->next;
            continue;
        }

        it = it->next;  // skip '#'
        struct list_t* stmt = get_preproc_stmt(&it, tk->ln);

        // ignore empty "#" stmt
        if (list_empty(stmt)) {
            continue;
        }

        const struct Token* kw = list_front(struct Token*, stmt);

        // #error
        if (streq(kw->raw, "error") || streq(kw->raw, "warning")) {
            int level = streq(kw->raw, "error") ? LEVEL_ERROR : LEVEL_WARNING;
            const struct Token* last = list_back(struct Token*, stmt);
            const int len = last->end - tk->start;
            error_tk(level, kw, "%.*s", len, tk->start);
            list_delete(stmt);
            continue;
        }

        // #include
        if (streq(kw->raw, "include")) {
            list_pop_front(struct Token*, stmt);
            if (list_empty(stmt)) {
                /// TODO: error recovery
                error_after_tk(LEVEL_ERROR, kw, "#include expects \"FILENAME\"");
                list_delete(stmt);
                continue;
            }

            const struct Token* filename = list_pop_front(struct Token*, stmt);
            if (filename->kind != TK_CSTR) {
                error_tk(LEVEL_ERROR, filename, "#include expects \"FILENAME\"");
                list_delete(stmt);
                continue;
            }

            const char* header_start = filename->start + 1;
            int header_len = filename->end - filename->start - 2;
            char header[MAX_PATH_LEN];
            snprintf(header, sizeof(header) - 1, "%.*s", header_len, header_start);

            // standard libary
            static const char* s_headers[] = {
                "stdarg.h",
                "stdio.h",
                "stdlib.h",
                "string.h",
            };

            const char* fullpath = NULL;
            for (size_t idx = 0; idx < ARRAY_LEN(s_headers); ++idx) {
                if (streq(s_headers[idx], header)) {
                    fullpath = path_concat("include/", header);
                    break;
                }
            }

            if (!fullpath) {
                fullpath = path_concat(filename->path, header);
            }

            struct FileCache* fcache = fcache_get(fullpath);
            if (fcache == NULL) {
                error_tk(LEVEL_FATAL, filename, "%s: No such file or directory", header);
            }

            for (struct list_node_t* it = fcache->rawtks->back; it; it = it->prev) {
                list_push_front(ntks, it->data);
            }

        } else {
            panic("TODO: implement #%s", kw->raw);
        }

        if (!list_empty(stmt)) {
            panic("TODO: warn on extra tokens at end of #include directive");
        }
        list_delete(stmt);
    }

    return ntks;
}
