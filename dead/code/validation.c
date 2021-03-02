#include "toyc.h"

// scope
typedef struct Scope Scope;
struct Scope {
    /// TODO: replace Int Dict with dict of declaration
    IntDict *varTable;
    eNode kind;
    int prevOffset;
    int offsetSoFar;
};

#define MAX_SCOPE 128
static Scope s_scopes[MAX_SCOPE];
static int s_scopeCnt = 0;

static Scope *push_scope(eNode kind) {
    assert(s_scopeCnt + 1 < MAX_SCOPE);
    Scope *scope = &s_scopes[s_scopeCnt];

    assert(!scope->varTable);
    scope->varTable = int_dict_make();
    scope->kind = kind;
    scope->prevOffset = 0;
    scope->offsetSoFar = 0;

    if (s_scopeCnt - 1 >= 0) {
        Scope *prevScope = &s_scopes[s_scopeCnt - 1];
        if (prevScope->kind == ND_COMP_STMT || prevScope->kind == ND_FOR) {
            scope->prevOffset = prevScope->prevOffset + prevScope->offsetSoFar;
        }
    }

    ++s_scopeCnt;
    return scope;
}

static void pop_scope() {
    assert(s_scopeCnt > 0);
    Scope *scope = &s_scopes[--s_scopeCnt];
    scope->kind = ND_INVALID;
    scope->prevOffset = 0;
    scope->offsetSoFar = 0;
    int_dict_destroy(&scope->varTable);
}

// control flow
static Node *s_currentFunc = NULL;
/// TODO: make a stack
static Node *s_loops[16];
static int s_loopCnt = 0;

static void push_loop(Node *loop) {
    assert(s_loopCnt < 16);
    s_loops[s_loopCnt] = loop;
    ++s_loopCnt;
}

static void pop_loop() {
    assert(s_loopCnt > 0);
    --s_loopCnt;
    s_loops[s_loopCnt] = NULL;
}

static Node *get_loop() {
    if (s_loopCnt - 1 < 0) {
        return NULL;
    }

    return s_loops[s_loopCnt - 1];
}

static void validate_trans_unit(Node *node) {
    for (NodeListNode *it = node->objList->begin; it; it = it->next) {
        validate(it->data);
    }
}

static void validate_var_decl(Node *node) {
    assert(s_scopeCnt);
    Scope *scope = &s_scopes[s_scopeCnt - 1];
    assert(scope->kind);

    const Token *token = node->token;
    if (int_dict_nhas(scope->varTable, token->loc.p, token->len)) {
        log_error_at_token(va("variable \"%s\" redifined", token_to_str(token)), token);
    }

    if (node->initializer) {
        assert(!node->expr);
        /// TODO: proper error message
        if (node->dataType->kind != DT_ARRAY) {
            assert(0);
        }

        if (node->dataType->arraySize == 0) {
            ((DataType *)node->dataType)->arraySize = node->initializer->size;
        } else if (node->dataType->arraySize < node->initializer->size) {
            assert(0);
        }

        for (NodeListNode *it = node->initializer->begin; it; it = it->next) {
            validate(it->data);
        }
    }

    if (node->expr) {
        assert(!node->initializer);
        validate(node->expr);
    }

    /// TODO: type check

    int sizeInByte = data_type_size(node->dataType);
    int_dict_ninsert(scope->varTable, token->loc.p, token->len, scope->offsetSoFar += sizeInByte);
}

static void validate_var(Node *node) {
    assert(s_scopeCnt);

    // find offset
    const Token *token = node->token;
    bool found = false;
    for (int i = s_scopeCnt - 1; i >= 0; --i) {
        Scope *scope = &s_scopes[i];
        if (int_dict_nget(scope->varTable, token->loc.p, token->len, &node->offset)) {
            node->offset += scope->prevOffset;
            // node->dataType =
            found = true;
            break;
        }
    }

    if (!found) {
        log_error_at_token(va("undefined symbol \"%s\"", token_to_str(token)), token);
    }
}

static void validate_func_impl(Node *node) {
    /// TODO: check if function redefined
    IntDict *varTable = push_scope(ND_FUNC_IMPL)->varTable;

    s_currentFunc = node;

    int offset = -8;
    for (NodeListNode *it = node->argList->begin; it; it = it->next) {
        const Token *token = it->data->token;
        bool exist = int_dict_nhas(varTable, token->loc.p, token->len);
        if (exist) {
            log_error_at_token("parameter redefinition", token);
        }

        int_dict_ninsert(varTable, token->loc.p, token->len, offset);
        // hard code 4 here
        int sizeInByte = data_type_size(it->data->dataType);
        offset -= sizeInByte;
    }

    validate(node->body);

    pop_scope();
}

static void validate_func_call(Node *node) {
    /// TODO: check types
    for (NodeListNode *it = node->argList->end; it; it = it->prev) {
        validate(it->data);
    }
}

static void validate_subscript(Node *node) {
    // check type of lhs, array or pointer
    validate(node->lhs);
    validate(node->rhs);
}

static void validate_comp_stmt(Node *node) {
    push_scope(ND_COMP_STMT);

    for (NodeListNode *it = node->stmtList->begin; it; it = it->next) {
        validate(it->data);
    }

    node->scopedMemory = s_scopes[s_scopeCnt - 1].offsetSoFar;
    pop_scope();
}

static void validate_expr_stmt(Node *node) {
    validate(node->expr);
}

static void validate_return_stmt(Node *node) {
    /// TODO: check if return type matches
    node->owner = s_currentFunc;
    validate(node->expr);
}

static void validate_continue_stmt(Node *node) {
    node->owner = get_loop();
    if (!node->owner) {
        log_error_at_token("continue statement not within loop", node->token);
    }
}

static void validate_break_stmt(Node *node) {
    node->owner = get_loop();
    if (!node->owner) {
        log_error_at_token("break statement not within loop", node->token);
    }
}

static void validate_if_stmt(Node *node) {
    validate(node->cond);
    validate(node->ifStmt);
    if (node->elseStmt) {
        validate(node->elseStmt);
    }
}

static void validate_ternary(Node *node) {
    validate(node->cond);
    validate(node->lhs);
    validate(node->rhs);
}

static void validate_while_stmt(Node *node) {
    push_loop(node);
    validate(node->cond);
    validate(node->body);
    pop_loop();
}

static void validate_for_stmt(Node *node) {
    push_loop(node);
    push_scope(ND_FOR);

    if (node->setup) {
        validate(node->setup);
    }

    if (node->cond) {
        validate(node->cond);
    }

    for (NodeListNode *it = node->increment->begin; it; it = it->next) {
        validate(it->data);
    }

    validate(node->body);

    node->scopedMemory = s_scopes[s_scopeCnt - 1].offsetSoFar;

    pop_scope();
    pop_loop();
}

static void validate_binary_expr(Node *node) {
    /// TODO: refactor this
    if (node->lhs->kind == ND_NUMBER && node->rhs->kind == ND_NUMBER) {
    }

    validate(node->lhs);
    validate(node->rhs);
    // check if type matches
    // node->sizeInByte = MAX(node->lhs->sizeInByte, node->rhs->sizeInByte);
}

static void validate_unary_expr(Node *node) {
    if (node->lhs) {
        validate(node->lhs);
    } else if (node->rhs) {
        validate(node->rhs);
    } else {
        assert(0);
    }
}

static void validate_assign(Node *node) {
    validate(node->lhs);
    validate(node->rhs);
    eNode kind = node->lhs->kind;
    if (kind != ND_VAR && kind != ND_DEREF && kind != ND_SUBSCRIPT) {
        log_error_at_token("expression must be lvalue", node->lhs->token);
    }
    /// TODO: check type, lvalue
}

void validate(Node *node) {
    assert(node);

    if (node->binary) {
        validate_binary_expr(node);
        return;
    }

    if (node->unary) {
        validate_unary_expr(node);
        return;
    }

    switch (node->kind) {
        /// TODO: refactor this
        case ND_NULL_EXPR:
            break;
        case ND_NUMBER:
            node->dataType = s_intType;
            node->sizeInByte = INT_SIZE_IN_BYTE;
            break;
        case ND_STRING:
            node->sizeInByte = POINTER_SIZE_IN_BYTE;
            break;
        case ND_EXPR_STMT:
            validate_expr_stmt(node);
            break;
        case ND_COMP_STMT:
            validate_comp_stmt(node);
            break;
        case ND_FUNC_CALL:
            validate_func_call(node);
            break;
        case ND_FUNC_IMPL:
            validate_func_impl(node);
            break;
        case ND_TRANS_UNIT:
            validate_trans_unit(node);
            break;
        case ND_RETURN:
            validate_return_stmt(node);
            break;
        case ND_VAR:
            validate_var(node);
            break;
        case ND_VAR_DECL:
            validate_var_decl(node);
            break;
        case ND_SUBSCRIPT:
            validate_subscript(node);
            break;
        case ND_ASSIGN:
            validate_assign(node);
            break;
        case ND_TERNARY:
            validate_ternary(node);
            break;
        case ND_IF_ELSE:
            validate_if_stmt(node);
            break;
        case ND_WHILE:
            validate_while_stmt(node);
            break;
        case ND_FOR:
            validate_for_stmt(node);
            break;
        case ND_BREAK:
            validate_break_stmt(node);
            break;
        case ND_CONTINUE:
            validate_continue_stmt(node);
            break;
        default:
            assertfmt(0, "Unhandled node type %s", eNode_to_str(node->kind));
            break;
    }
}
