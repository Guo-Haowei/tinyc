#include "cc.h"

static struct Node* node_new(int kind);
static struct Node* stmt_node_new(int kind, struct Node* parent);

static struct list_node_t* g_iter;

static struct map_t* g_funcs;

static struct Token* peek();
static struct Token* pop();

static struct Token* try_token(int kind);
static struct Token* expect_token(int kind);

static struct Symbol* symbol_new(int kind);
static struct Symbol* global_decl();

static struct DataType* parse_type();

static struct Node* stmt(struct Node* parent);
static struct Node* stmt_cmp(struct Node* parent);
static struct Node* stmt_ret(struct Node* parent);

static struct Node* expr();
static struct Node* expr_literal();

/// parser
static struct Symbol* global_decl() {
    /// TODO: storage
    struct DataType* data_type = parse_type();
    struct Token* symbol = expect_token(TK_SYMBOL);

    if (try_token(TK_SC)) {
        panic("TODO: support global var");
    }

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
    func->retType = data_type;
    func->symbol = symbol;
    func->params = params;

    struct map_pair_t* pair = map_find(g_funcs, symbol->raw);
    if (pair) {
        panic("TODO: handle case when function already declared/defined");
    }

    map_insert(g_funcs, symbol->raw, func);

    dumpfunc(stderr, func);

    // declaration
    if (peek()->kind == TK_SC) {
        /// TODO: func node
        panic("TODO!!!");
    }

    struct Node* body = stmt_cmp(NULL);
    dumpnode(stderr, body);

    return func;
}

// <statement> ::= <identifier> : <statement>
//               | case <constant-expression> : <statement>
//               | default : <statement>
//               | {<expression>}? ;
//               | { {<declaration>}* {<statement>}* }
//               | if ( <expression> ) <statement>
//               | if ( <expression> ) <statement> else <statement>
//               | switch ( <expression> ) <statement>
//               | while ( <expression> ) <statement>
//               | do <statement> while ( <expression> ) ;
//               | for ( {<expression>}? ; {<expression>}? ; {<expression>}? ) <statement>
//               | goto <identifier> ;
//               | continue ;
//               | break ;
//               | return {<expression>}? ;
//               | <compound-statement>
static struct Node* stmt(struct Node* parent) {
    struct Token* tk = peek();
    const int tk_kind = tk->kind;
    if (tk_kind == TK_LBR) {
        return stmt_cmp(parent);
    }

    if (tk_kind == TK_KW_RETURN) {
        return stmt_ret(parent);
    }

    panic("TODO: implement the stmt()");
    return NULL;
}

// <compound-statement> ::= { {<declaration>}* {<statement>}* }
static struct Node* stmt_cmp(struct Node* parent) {
    struct Node* cmp = stmt_node_new(ND_STMT_CMP, parent);
    cmp->begin = expect_token(TK_LBR);
    cmp->stmts = list_new();

    /// TODO: symbol table

    while (peek()->kind != TK_RBR) {
        struct Node* statment = stmt(cmp);
        list_push_back(cmp->stmts, statment);
    }

    cmp->end = expect_token(TK_RBR);
    return cmp;
}

static struct Node* stmt_ret(struct Node* parent) {
    struct Node* ret = stmt_node_new(ND_STMT_RET, parent);
    ret->begin = expect_token(TK_KW_RETURN);
    if (peek()->kind != TK_SC) {
        ret->expr = expr(NULL);
    }
    ret->end = expect_token(TK_SC);
    return ret;
}

static struct Node* expr() {
    return expr_literal();
}

static struct Node* expr_literal() {
    struct Token* tk = pop();

    struct Node* literal = node_new(ND_EXPR_LIT);
    literal->begin = tk;
    literal->end = tk;

    if (tk->kind == TK_CINT) {
        literal->ivalue = atoi(tk->raw);
        literal->type = g_int;
    } else {
        panic("TODO: implement literal");
    }
    return literal;
}

struct Node* parse(struct list_t* tks) {
    // initialize function table
    if (g_funcs) {
        map_delete(g_funcs);
    }
    g_funcs = map_new();

    g_iter = tks->front;

    expect_token(TK_BEGIN);

    while (peek()->kind != TK_END) {
        global_decl();
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
    int size = 0;

    bool mutable = !try_token(TK_KW_CONST);

    tk = pop();
    if (tk->kind == TK_KW_INT) {
        type = DT_INT;
        size = INT_SIZE_IN_BYTE;
    } else if (tk->kind == TK_KW_CHAR) {
        type = DT_CHAR;
        size = 1;
    } else if (tk->kind == TK_KW_VOID) {
        type = DT_VOID;
    } else {
        panic("TODO: handle invalid type");
    }

    struct DataType* dt = datatype_new(type, size, mutable);

    while ((tk = try_token(TK_STAR))) {
        mutable = !try_token(TK_KW_CONST);
        struct DataType* base = dt;
        dt = datatype_new(DT_PTR, PTR_SIZE_IN_BYTE, mutable);
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

static struct Node* node_new(int kind) {
    cassert(kind > ND_INVALID && kind < ND_COUNT);
    struct Node* node = alloct(sizeof(struct Node));
    memset(node, 0, sizeof(struct Node));
    node->kind = kind;
    return node;
}

static struct Node* stmt_node_new(int kind, struct Node* parent) {
    struct Node* node = node_new(kind);
    node->parent = parent;
    return node;
}
