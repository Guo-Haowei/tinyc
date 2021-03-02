#include "toyc.h"

/// TODO: refactor this
static int s_escapeTable[128];
static bool s_escapeTableInit = false;

// https://stackoverflow.com/questions/7666509/hash-function-for-string
unsigned int hash_str(const char *str, int length) {
    unsigned int hash = 5381;
    for (int i = 0; i < length; ++i) {
        hash = ((hash << 5) + hash) + str[i];  // hash * 33 + c
    }

    return hash;
}

void log(char const *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

char const *va(char const *format, ...) {
    static char s_buffer[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(s_buffer, sizeof(s_buffer), format, args);
    va_end(args);
    return s_buffer;
}

char const *token_to_str(Token const *tk) {
    static char s_buffer[1024];
    assertfmt(tk->len + 1 < sizeof(s_buffer), "token too long");
    strncpy(s_buffer, tk->loc.p, tk->len);
    s_buffer[tk->len] = '\0';
    return s_buffer;
}

char const *find_line(Loc const *loc) {
    static char s_line[1024];
    char const *p = loc->fileCache->content;
    for (int i = 1; i < loc->ln; ++i) {
        p = strchr(p, '\n');
        assert(p);
        ++p;
    }

    strncpy(s_line, p, sizeof(s_line));

    char *pp;
    if (pp = strchr(s_line, '\n')) {
        *pp = '\0';
    }

    return s_line;
}

int escaped(int c) {
    assert(0 <= c && c < 128);

    if (!s_escapeTableInit) {
        for (int i = 0; i < ARRAY_SIZE(s_escapeTable); ++i) {
            s_escapeTable[i] = INVALID_ESCAPE;
        }

        s_escapeTable['\''] = '\'';
        s_escapeTable['\"'] = '\"';
        s_escapeTable['\?'] = '\?';
        s_escapeTable['\\'] = '\\';

        s_escapeTable['a'] = '\a';
        s_escapeTable['b'] = '\b';
        s_escapeTable['f'] = '\f';
        s_escapeTable['n'] = '\n';
        s_escapeTable['r'] = '\r';
        s_escapeTable['t'] = '\t';

        s_escapeTableInit = true;
    }

    return s_escapeTable[c];
}

char const *eToken_to_str(eToken kind) {
    static char const *tokenNameLut[TK_COUNT] = {
        "TK_INVALID",
        "TK_ID",
        "TK_KEYWORD",
        "TK_PUNCT",
        "TK_NUMBER",
        "TK_STRING",
        "TK_CHAR",
    };

    return tokenNameLut[kind];
}

char const *eNode_to_str(eNode kind) {
    static char const *nodeNameLut[ND_COUNT + 1] = {
        "ND_INVALID",
        "ND_NULL_EXPR",
        "ND_FUNC_DECL",
        "ND_FUNC_IMPL",
        "ND_VAR_DECL",
        "ND_VAR",
        "ND_ASSIGN",
        "ND_POS",
        "ND_NEG",
        "ND_NOT",
        "ND_ADDRESS",
        "ND_DEREF",
        "ND_OR",
        "ND_AND",
        "ND_EQ",
        "ND_NE",
        "ND_GT",
        "ND_GE",
        "ND_LT",
        "ND_LE",
        "ND_ADD",
        "ND_SUB",
        "ND_MUL",
        "ND_DIV",
        "ND_REM",
        "ND_INC_I",
        "ND_DEC_I",
        "ND_I_INC",
        "ND_I_DEC",
        "ND_SUBSCRIPT",
        "ND_NUMBER",
        "ND_STRING",
        "ND_FUNC_CALL",
        "ND_RETURN",
        "ND_BREAK",
        "ND_CONTINUE",
        "ND_IF_ELSE",
        "ND_TERNARY",
        "ND_WHILE",
        "ND_FOR",
        "ND_DO_WHILE",
        "ND_EXPR_STMT",
        "ND_COMP_STMT",
        "ND_TRANS_UNIT",
        NULL,
    };

    return nodeNameLut[kind];
}
#define INDENT_BLANK "                                        "

static void debug_print_node_internal(Node const *node, int indent);

static void debug_print_node_list(NodeList const *nodeList, int indent) {
    for (NodeListNode *it = nodeList->begin; it; it = it->next) {
        debug_print_node_internal(it->data, indent);
        if (it != nodeList->end) {
            log(", ");
        }
    }
}

static void debug_print_var_decl_internal(Node const *node, int indent) {
    log("%s %s", data_type_to_string(node->dataType), token_to_str(node->token));
    if (node->expr) {
        log(" = ");
        debug_print_node_internal(node->expr, indent);
    } else if (node->initializer) {
        log(" = { ");
        for (NodeListNode *it = node->initializer->begin; it; it = it->next) {
            if (it != node->initializer->begin) {
                log(", ");
            }
            debug_print_node_internal(it->data, indent);
        }
        log(" }");
    }
}

static void debug_print_node_internal(Node const *node, int indent) {
    assert(node);
    if (node->binary) {
        log("(");
        debug_print_node_internal(node->lhs, indent);
        log(" %s ", token_to_str(node->token));
        debug_print_node_internal(node->rhs, indent);
        log(")");
        return;
    }

    /// TODO: refactor this
    if (node->unary && node->kind != ND_I_DEC && node->kind != ND_I_INC) {
        log("%s(", token_to_str(node->token));
        debug_print_node_internal(node->rhs, indent);
        log(")");
        return;
    }

    switch (node->kind) {
        case ND_NULL_EXPR:
            break;
        case ND_FUNC_IMPL:
            log("%s %s(", data_type_to_string(node->dataType), token_to_str(node->token));
            for (NodeListNode *it = node->argList->begin; it; it = it->next) {
                if (it != node->argList->begin) {
                    log(", ");
                }
                log("%s %s", data_type_to_string(it->data->dataType), token_to_str(it->data->token));
            }
            log(")\n");
            debug_print_node_internal(node->body, indent);
            break;
        case ND_VAR:
            // log("%s", token_to_str(node->token));
            log("%s(%d)", token_to_str(node->token), node->offset);
            break;
        case ND_NUMBER:
            log("%s", token_to_str(node->token));
            break;
        case ND_STRING:
            log("\"%s\"", token_to_str(node->token));
            break;
        case ND_SUBSCRIPT:
            debug_print_node_internal(node->lhs, indent);
            log("[");
            debug_print_node_internal(node->rhs, indent);
            log("]");
            break;
        case ND_TERNARY:
            log("(");
            debug_print_node_internal(node->cond, indent);
            log(" ? ");
            debug_print_node_internal(node->lhs, indent);
            log(" : ");
            debug_print_node_internal(node->rhs, indent);
            log(")");
            break;
        case ND_FUNC_CALL:
            log("%s(", token_to_str(node->identifier->token));
            debug_print_node_list(node->argList, indent);
            log(")");
            break;
        case ND_VAR_DECL:
            log("%.*s", indent, INDENT_BLANK);
            debug_print_var_decl_internal(node, indent);
            log(";\n");
            break;
        case ND_ASSIGN:
            debug_print_node_internal(node->lhs, indent);
            log(" = ");
            debug_print_node_internal(node->rhs, indent);
            break;
        case ND_BREAK:
        case ND_CONTINUE:
            log("%.*s%.*s;\n", indent, INDENT_BLANK, node->token->len, node->token->loc.p);
            break;
        case ND_RETURN:
            log("%.*sreturn ", indent, INDENT_BLANK);
            debug_print_node_internal(node->expr, indent);
            log(";\n");
            break;
        case ND_EXPR_STMT:
            log("%.*s", indent, INDENT_BLANK);
            debug_print_node_internal(node->expr, indent);
            log(";\n");
            break;
        case ND_COMP_STMT:
            log("%.*s{\n", indent, INDENT_BLANK);
            for (NodeListNode *it = node->stmtList->begin; it; it = it->next) {
                debug_print_node_internal(it->data, indent + 4);
            }
            log("%.*s}\n", indent, INDENT_BLANK);
            break;
        case ND_IF_ELSE:
            log("%.*sif (", indent, INDENT_BLANK);
            debug_print_node_internal(node->cond, indent);
            log(")\n");
            debug_print_node_internal(node->ifStmt, node->ifStmt->kind == ND_COMP_STMT ? indent : indent + 4);
            if (node->elseStmt) {
                log("%.*selse\n", indent, INDENT_BLANK);
                debug_print_node_internal(node->elseStmt, node->elseStmt->kind == ND_COMP_STMT ? indent : indent + 4);
            }
            break;
        case ND_WHILE:
            log("%.*swhile (", indent, INDENT_BLANK);
            debug_print_node_internal(node->cond, indent);
            log(")\n");
            debug_print_node_internal(node->body, node->body->kind == ND_COMP_STMT ? indent : indent + 4);
            break;
        case ND_FOR:
            log("%.*sfor (", indent, INDENT_BLANK);
            if (node->setup) {
                assert(node->setup->kind == ND_VAR_DECL);
                debug_print_var_decl_internal(node->setup, indent);
            }
            log(" ; ");
            if (node->cond) {
                debug_print_node_internal(node->cond, indent);
            }
            log(" ; ");
            assert(node->increment);
            debug_print_node_list(node->increment, indent);
            log(")\n");
            debug_print_node_internal(node->body, node->body->kind == ND_COMP_STMT ? indent : indent + 4);
            break;
        case ND_TRANS_UNIT:
            for (NodeListNode *n = node->objList->begin; n; n = n->next) {
                debug_print_node_internal(n->data, indent);
            }
            break;
        case ND_I_INC:
        case ND_I_DEC:
            debug_print_node_internal(node->lhs, indent);
            log("%s", token_to_str(node->token));
            break;
        default:
            assertfmt(0, "Unhandled node type %s", eNode_to_str(node->kind));
            break;
    }
}

void debug_print_node(Node const *node) {
    debug_print_node_internal(node, 0);
}

void debug_print_token(TokenList const *tokenList) {
    for (TokenListNode const *it = tokenList->begin; it; it = it->next) {
        Token *t = it->data;
        log(
            "%s [ln:%3d, col:%3d] %-10s %s\n",
            t->loc.fileCache->path,
            t->loc.ln,
            t->loc.col,
            eToken_to_str(t->kind),
            token_to_str(t));
    }
}

#define T int
#define T_NAME(x) Int##x
#define T_FUNC_NAME(x) int_##x
#include "containers/dict_impl.h"
#undef T
#undef T_NAME
#undef T_FUNC_NAME

void debug_print_int_dict(IntDict *const dict) {
    assert(dict);

    log("dict has %d entries\n", dict->size);
    for (int i = 0; i < dict->capacity; ++i) {
        for (IntDictNode *n = dict->bucket[i]; n; n = n->next) {
            log("[%.*s]: %d\n", n->keyLen, n->key, n->data);
        }
    }
}
