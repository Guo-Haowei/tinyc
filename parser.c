#include "cc.h"

static struct list_node_t* g_iter;

static struct Token* peek();
static struct Token* pop();

static struct Token* try_token(int kind);
static struct Token* expect_token(int kind);

static struct DataType* parse_type();

static struct Symbol* symbol_new(int kind);

/// parser
static void parse_func_decl() {
    /// TODO: storage
    struct DataType* ret_type = parse_type();
    struct Token* symbol = expect_token(TK_SYMBOL);
    expect_token(TK_LBK);
    struct list_t* params = list_new();

    while (peek()->kind != TK_RBK) {
        if (list_len(params)) {
            expect_token(TK_COMMA);
        }

        struct Symbol* param = symbol_new(SYMBOL_VAR);
        param->dataType = parse_type();
        param->symbol = expect_token(TK_SYMBOL);
        list_push_back(params, param);
    }

    expect_token(TK_RBK);

    struct Symbol* func = symbol_new(SYMBOL_FUNC);
    func->retType = ret_type;
    func->symbol = symbol;
    func->params = params;

    dumpfunc(stderr, func);
    fprintf(stderr, "\n");
    
    exit(1);
}

struct Node* parse(struct list_t* tks) {
    g_iter = tks->front;

    expect_token(TK_BEGIN);

    while (g_iter) {
        parse_func_decl();
    }

    return NULL;
}

static struct Token* peek() {
    cassert(g_iter);
    return CAST(struct Token*, g_iter->data);
}

static struct Token* pop() {
    struct Token* tk = peek();
    g_iter = g_iter->next;
    return tk;
}

static struct Token* try_token(int kind) {
    struct Token* tk = peek();
    if (tk->kind == kind) {
        pop();
        return tk;
    }

    return NULL;
}

static struct Token* expect_token(int kind) {
    struct Token* tk = pop();
    if (!tk) {
        panic("Unexpected end of file\n");
    }

    if (tk->kind != kind) {
        error_tk(LEVEL_FATAL, tk, "expected \"%s\", got \"%s\"", tk2prettystr(kind), tk->raw);
    }

    return tk;
}

static struct DataType* parse_type() {
    struct Token* tk = NULL;
    int type = DT_INVALID;

    bool mutable = !try_token(TK_KW_CONST);

    tk = pop();
    if (tk->kind == TK_KW_INT) {
        type = DT_INT;
    } else if (tk->kind == TK_KW_CHAR) {
        type = DT_CHAR;
    } else if (tk->kind == TK_KW_VOID) {
        type = DT_VOID;
    } else {
        panic("TODO: handle invalid type");
    }

    struct DataType* dt = datatype_new(type, mutable);

    while ((tk = try_token(TK_STAR))) {
        mutable = !try_token(TK_KW_CONST);
        struct DataType* base = dt;
        dt = datatype_new(DT_PTR, mutable);
        dt->base = base;
    }

    return dt;
}

static struct Symbol* symbol_new(int kind) {
    cassert(kind > SYMBOL_INVALID && kind < SYMBOL_COUNT);
    struct Symbol* symbol = alloct(sizeof(struct Symbol));
    /// TODO: set scope name and id

    symbol->kind = kind;

    symbol->retType = NULL;
    symbol->params = NULL;
    symbol->definition = NULL;

    symbol->dataType = NULL;

    return symbol;
}
