#include "cc.h"

void init_preproc() {
}

void shutdown_preproc() {
}

static struct list_t* get_preproc_stmt(struct list_node_t** it, int ln) {
    list_new(stmt);

    while (*it) {
        const struct Token* tk = (*it)->data;
        if (tkeqc(tk, '\\')) {
            panic("todo");
        }

        assert(tk->ln >= ln);
        if (tk->ln != ln) {
            break;
        }

        list_push_back(stmt, tk);

        *it = (*it)->next;
    }

    return stmt;
}

struct list_t* preproc(struct list_t* tks) {
    list_new(ntks);

    struct list_node_t* it = tks->front;
    while (it) {
        const struct Token* tk = it->data;

        /// TODO: refactor this
        bool is_preproc = false;
        if (tkeqc(tk, '#')) {
            is_preproc = true;
            if (it->prev) {
                const struct Token* ptk = it->prev->data;
                if (tk->ln == ptk->ln) {
                    is_preproc = false;
                }
            }
        }

        if (!is_preproc) {
            int len = tk->end - tk->start;
            assert(len > 0);
            fprintf(stderr,
                    "[%7s] [f: " ANSI_GREEN "\"%s\"" ANSI_RESET ", ln: %2d, col: %2d]",
                    tk2str(tk->kind),
                    tk->path,
                    tk->ln,
                    tk->col);
            fprintf(stderr, " [\e[1;31m%.*s\e[0m]\n", len, tk->start);
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
        const int kwlen = kw->end - kw->start;

        if (tkeqstr(kw, "error") || tkeqstr(kw, "warning")) {
            int level = tkeqstr(kw, "error") ? LEVEL_ERROR : LEVEL_WARNING;
            const struct Token* last = list_back(struct Token*, stmt);
            const int len = last->end - tk->start;
            error_tk(level, kw, "%.*s", len, tk->start);
            list_delete(stmt);
            continue;
        }

        if (tkeqstr(kw, "include")) {
        } else {
            debugln("unknow preprocessor directive #%.*s", kwlen, kw->start);
            panic("TODO: implement");
        }

        if (!list_empty(stmt)) {
            panic("TODO: warn on extra tokens at end of #include directive");
        }
        list_delete(stmt);
    }

    return ntks;
}
