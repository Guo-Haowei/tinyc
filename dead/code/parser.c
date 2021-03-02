/*
<translation-unit> ::= {<external-declaration>}*

<external-declaration> ::= <function-definition>
                         | <declaration>

<function-definition> ::= {<declaration-specifier>}* <declarator> {<declaration>}* <compound-statement>

<declaration-specifier> ::= <storage-class-specifier>
                          | <type-specifier>
                          | <type-qualifier>

<storage-class-specifier> ::= auto
                            | register
                            | static
                            | extern
                            | typedef

<type-specifier> ::= void
                   | char
                   | short
                   | int
                   | long
                   | float
                   | double
                   | signed
                   | unsigned
                   | <struct-or-union-specifier>
                   | <enum-specifier>
                   | <typedef-name>

<struct-or-union-specifier> ::= <struct-or-union> <identifier> { {<struct-declaration>}+ }
                              | <struct-or-union> { {<struct-declaration>}+ }
                              | <struct-or-union> <identifier>

<struct-or-union> ::= struct
                    | union

<struct-declaration> ::= {<specifier-qualifier>}* <struct-declarator-list>

<specifier-qualifier> ::= <type-specifier>
                        | <type-qualifier>

<struct-declarator-list> ::= <struct-declarator>
                           | <struct-declarator-list> , <struct-declarator>

<struct-declarator> ::= <declarator>
                      | <declarator> : <constant-expression>
                      | : <constant-expression>

<declarator> ::= {<pointer>}? <direct-declarator>

<pointer> ::= * {<type-qualifier>}* {<pointer>}?

<type-qualifier> ::= const
                   | volatile

<direct-declarator> ::= <identifier>
                      | ( <declarator> )
                      | <direct-declarator> [ {<constant-expression>}? ]
                      | <direct-declarator> ( <parameter-type-list> )
                      | <direct-declarator> ( {<identifier>}* )

<constant-expression> ::= <conditional-expression>

<inclusive-or-expression> ::= <exclusive-or-expression>
                            | <inclusive-or-expression> | <exclusive-or-expression>

<exclusive-or-expression> ::= <and-expression>
                            | <exclusive-or-expression> ^ <and-expression>

<and-expression> ::= <equality-expression>
                   | <and-expression> & <equality-expression>

<equality-expression> ::= <relational-expression>
                        | <equality-expression> == <relational-expression>
                        | <equality-expression> != <relational-expression>

<relational-expression> ::= <shift-expression>
                          | <relational-expression> < <shift-expression>
                          | <relational-expression> > <shift-expression>
                          | <relational-expression> <= <shift-expression>
                          | <relational-expression> >= <shift-expression>

<shift-expression> ::= <additive-expression>
                     | <shift-expression> << <additive-expression>
                     | <shift-expression> >> <additive-expression>

<type-name> ::= {<specifier-qualifier>}+ {<abstract-declarator>}?

<parameter-type-list> ::= <parameter-list>
                        | <parameter-list> , ...

<parameter-list> ::= <parameter-declaration>
                   | <parameter-list> , <parameter-declaration>

<parameter-declaration> ::= {<declaration-specifier>}+ <declarator>
                          | {<declaration-specifier>}+ <abstract-declarator>
                          | {<declaration-specifier>}+

<abstract-declarator> ::= <pointer>
                        | <pointer> <direct-abstract-declarator>
                        | <direct-abstract-declarator>

<direct-abstract-declarator> ::=  ( <abstract-declarator> )
                               | {<direct-abstract-declarator>}? [ {<constant-expression>}? ]
                               | {<direct-abstract-declarator>}? ( {<parameter-type-list>|? )

<enum-specifier> ::= enum <identifier> { <enumerator-list> }
                   | enum { <enumerator-list> }
                   | enum <identifier>

<enumerator-list> ::= <enumerator>
                    | <enumerator-list> , <enumerator>

<enumerator> ::= <identifier>
               | <identifier> = <constant-expression>

<typedef-name> ::= <identifier>

<declaration> ::=  {<declaration-specifier>}+ {<init-declarator>}* ;

<init-declarator> ::= <declarator>
                    | <declarator> = <initializer>

<initializer> ::= <assignment-expression>
                | { <initializer-list> }
                | { <initializer-list> , }

<initializer-list> ::= <initializer>
                     | <initializer-list> , <initializer>

*/

#include "toyc.h"

static int g_node_counter = 0;

static Node *make_empty_node(eNode kind, Token const *token) {
    assert(token);

    Node *node = CALLOC(sizeof(Node));
    node->kind = kind;
    node->guid = ++g_node_counter;
    node->token = token;
    return node;
}

bool match_str(Token const *token, char const *str) {
    assert(token);
    return strlen(str) == token->len && strncmp(str, token->loc.p, token->len) == 0;
}

bool match_char(Token const *token, char c) {
    assert(token);
    return token->len == 1 && *token->loc.p == c;
}

void expect_and_pop_char_punct(TokenList *tokenList, char c) {
    Token *tk = token_list_pop_front(tokenList);
    if (!match_char(tk, c)) {
        log_error_at_token(
            va("unexpected token \"%s\", expect \"%c\"", token_to_str(tk), c),
            tk);
    }
}

/// forward declaration
static Node *parse_primary_expr(TokenList *tokenList);
static Node *parse_assign_expr(TokenList *tokenList);
static Node *parse_ternary_expr(TokenList *tokenList);
static Node *parse_or_expr(TokenList *tokenList);
static Node *parse_and_expr(TokenList *tokenList);
static Node *parse_equality_expr(TokenList *tokenList);
static Node *parse_relational_expr(TokenList *tokenList);
static Node *parse_add_expr(TokenList *tokenList);
static Node *parse_mul_expr(TokenList *tokenList);
static Node *parse_cast_expr(TokenList *tokenList);
static Node *parse_unary_expr(TokenList *tokenList);
static Node *parse_postfix_expr(TokenList *tokenList);
static Node *parse_expr(TokenList *tokenList);

static Node *parse_comp_stmt(TokenList *tokenList);
static Node *parse_stmt(TokenList *tokenList);

// <assignment-expression> ::= <conditional-expression>
//                           | <unary-expression> = <assignment-expression>
Node *parse_assign_expr(TokenList *tokenList) {
    Node *ret = parse_ternary_expr(tokenList);
    Token *tk = token_list_front(tokenList);
    if (match_char(tk, '=')) {
        token_list_pop_front(tokenList);  // pop "="
        Node *lhs = ret;
        ret = make_empty_node(ND_ASSIGN, tk);
        ret->lhs = lhs;
        ret->rhs = parse_assign_expr(tokenList);
    }

    return ret;
}

// <conditional-expression> ::= <logical-or-expression>
//                            | <logical-or-expression> ? <expression> : <conditional-expression>
Node *parse_ternary_expr(TokenList *tokenList) {
    Node *cond = parse_or_expr(tokenList);
    Token const *tk = token_list_front(tokenList);

    if (!match_char(tk, '?')) {
        return cond;
    }

    token_list_pop_front(tokenList);  // pop "?"
    Node *ternary = make_empty_node(ND_TERNARY, tk);
    ternary->cond = cond;
    ternary->lhs = parse_expr(tokenList);
    expect_and_pop_char_punct(tokenList, ':');
    ternary->rhs = parse_expr(tokenList);
    return ternary;
}

#define IMPLEMENT_BINARY_INTERNAL(func1, func2, kind)  \
    Node *func1(TokenList *tokenList) {                \
        Node *ret = func2(tokenList);                  \
        Token const *tk = token_list_front(tokenList); \
        for (;;) {                                     \
            const eNode nodeKind = kind;               \
            if (nodeKind == ND_INVALID)                \
                break;                                 \
            token_list_pop_front(tokenList);           \
            Node *lhs = ret;                           \
            ret = make_empty_node(nodeKind, tk);       \
            ret->binary = true;                        \
            ret->lhs = lhs;                            \
            ret->rhs = func2(tokenList);               \
            tk = token_list_front(tokenList);          \
        }                                              \
        return ret;                                    \
    }

// <logical-or-expression> ::= <logical-and-expression>
//                           | <logical-or-expression || <logical-and-expression>
IMPLEMENT_BINARY_INTERNAL(
    parse_or_expr,
    parse_and_expr,
    match_str(tk, "||") ? ND_OR : ND_INVALID)

// <logical-and-expression> ::= <inclusive-or-expression>
//                            | <logical-and-expression && <inclusive-or-expression>
IMPLEMENT_BINARY_INTERNAL(
    parse_and_expr,
    parse_equality_expr,
    match_str(tk, "&&") ? ND_AND : ND_INVALID)

// <equality-expression> ::= <relational-expression>
//                         | <equality-expression> == <relational-expression>
//                         | <equality-expression> != <relational-expression>
IMPLEMENT_BINARY_INTERNAL(
    parse_equality_expr,
    parse_relational_expr,
    (match_str(tk, "==")) ? ND_EQ : (match_str(tk, "!=")) ? ND_NE
                                                          : ND_INVALID)
// <relational-expression> ::= <shift-expression>
//                           | <relational-expression> < <shift-expression>
//                           | <relational-expression> > <shift-expression>
//                           | <relational-expression> <= <shift-expression>
//                           | <relational-expression> >= <shift-expression>
IMPLEMENT_BINARY_INTERNAL(
    parse_relational_expr,
    parse_add_expr,
    (match_char(tk, '>')) ? ND_GT : (match_char(tk, '<')) ? ND_LT
                                : (match_str(tk, ">="))   ? ND_GE
                                : (match_str(tk, "<="))   ? ND_LE
                                                          : ND_INVALID)
// <additive-expression> ::= <multiplicative-expression>
//                         | <additive-expression> + <multiplicative-expression>
//                         | <additive-expression> - <multiplicative-expression>
IMPLEMENT_BINARY_INTERNAL(
    parse_add_expr,
    parse_mul_expr,
    (match_char(tk, '+')) ? ND_ADD : (match_char(tk, '-')) ? ND_SUB
                                                           : ND_INVALID)
// <multiplicative-expression> ::= <cast-expression>
//                               | <multiplicative-expression> * <cast-expression>
//                               | <multiplicative-expression> / <cast-expression>
//                               | <multiplicative-expression> % <cast-expression>
IMPLEMENT_BINARY_INTERNAL(
    parse_mul_expr,
    parse_cast_expr,
    (match_char(tk, '*')) ? ND_MUL : (match_char(tk, '/')) ? ND_DIV
                                 : (match_char(tk, '%'))   ? ND_REM
                                                           : ND_INVALID)

static NodeList *parse_expr_list(TokenList *tokenList) {
    NodeList *exprList = node_list_make();
    if (match_char(token_list_front(tokenList), ')')) {
        return exprList;
    }

    node_list_push_back(exprList, parse_expr(tokenList));

    while (1) {
        Token *tk = token_list_front(tokenList);
        if (match_char(tk, ')')) {
            break;
        }

        if (!match_char(tk, ',')) {
            log_unexpected_token(tk);
        }

        token_list_pop_front(tokenList);  // pop ","
        node_list_push_back(exprList, parse_expr(tokenList));
    }

    return exprList;
}

// <cast-expression> ::= <unary-expression>
//                     | ( <type-name> ) <cast-expression>
Node *parse_cast_expr(TokenList *tokenList) {
    return parse_unary_expr(tokenList);
}

// <unary-expression> ::= <postfix-expression>
//                      | ++ <unary-expression>
//                      | -- <unary-expression>
//                      | &,*,+,-,~,! <cast-expression>
//                      | sizeof <unary-expression>
//                      | sizeof <type-name>
Node *parse_unary_expr(TokenList *tokenList) {
    Token *tk = token_list_front(tokenList);
    eNode kind = match_char(tk, '!') ? ND_NOT : match_char(tk, '-') ? ND_NEG
                                            : match_char(tk, '+')   ? ND_POS
                                            : match_char(tk, '*')   ? ND_DEREF
                                            : match_char(tk, '&')   ? ND_ADDRESS
                                                                    : ND_INVALID;
    if (kind != ND_INVALID) {
        token_list_pop_front(tokenList);
        Node *unary = make_empty_node(kind, tk);
        unary->rhs = parse_cast_expr(tokenList);
        unary->unary = true;
        return unary;
    }

    kind = match_str(tk, "++") ? ND_INC_I : match_str(tk, "--") ? ND_DEC_I
                                                                : ND_INVALID;
    if (kind != ND_INVALID) {
        token_list_pop_front(tokenList);
        Node *unary = make_empty_node(kind, tk);
        unary->rhs = parse_unary_expr(tokenList);
        unary->unary = true;
        return unary;
    }

    return parse_postfix_expr(tokenList);
}

// <postfix-expression> ::= <primary-expression>
//                        | <postfix-expression> [ <expression> ]
//                        | <postfix-expression> ( {expression, }* )
//                        | <postfix-expression> . <identifier>
//                        | <postfix-expression> -> <identifier>
//                        | <postfix-expression> ++
//                        | <postfix-expression> --
Node *parse_postfix_expr(TokenList *tokenList) {
    Node *ret = parse_primary_expr(tokenList);

    for (Token *next = token_list_front(tokenList);; next = token_list_front(tokenList)) {
        const eNode kind = match_char(next, '(') ? ND_FUNC_CALL : match_str(next, "++") ? ND_I_INC
                                                              : match_str(next, "--")   ? ND_I_DEC
                                                              : match_char(next, '[')   ? ND_SUBSCRIPT
                                                                                        : ND_INVALID;
        if (kind == ND_INVALID) {
            break;
        }

        token_list_pop_front(tokenList);
        if (kind == ND_FUNC_CALL) {
            Node *node = make_empty_node(ND_FUNC_CALL, next);
            node->identifier = ret;
            node->argList = parse_expr_list(tokenList);
            ret = node;
            expect_and_pop_char_punct(tokenList, ')');
            continue;
        }

        if (kind == ND_I_INC || kind == ND_I_DEC) {
            Node *node = make_empty_node(kind, next);
            node->lhs = ret;
            node->unary = true;
            ret = node;
            continue;
        }

        if (kind == ND_SUBSCRIPT) {
            Node *node = make_empty_node(ND_SUBSCRIPT, next);
            node->lhs = ret;
            node->rhs = parse_expr(tokenList);
            ret = node;
            expect_and_pop_char_punct(tokenList, ']');
            continue;
        }
    }

    return ret;
}

// <expression> ::= <assignment-expression>
//                | <expression> , <assignment-expression>
Node *parse_expr(TokenList *tokenList) {
    return parse_assign_expr(tokenList);
}

// <primary-expression> ::= <identifier>
//                        | <integer-constant>
//                        | <character-constant>
//                        | <floating-constant>
//                        | <enumeration-constant>
//                        | <string-literal>
//                        | ( <expression> )
Node *parse_primary_expr(TokenList *tokenList) {
    Token *tk = token_list_pop_front(tokenList);
    if (tk->kind == TK_NUMBER) {
        Node *node = make_empty_node(ND_NUMBER, tk);
        node->intVal = atoi(tk->loc.p);
        return node;
    }

    if (tk->kind == TK_STRING) {
        Node *node = make_empty_node(ND_STRING, tk);
        /// NOTE: parse string
        char *buf = CALLOC(strlen(tk->loc.p));
        node->strVal = buf;
        char const *p = tk->loc.p;
        int i = 0;
        while (i < tk->len) {
            if (*p == '\\') {
                ++p;
                ++i;
                char c = escaped(*p);
                if (c == INVALID_ESCAPE) {
                    log_error_at_token(va("unknown escape sequence: '\\%c'", *p), tk);
                }
                *buf = c;
            } else {
                *buf = *p;
            }
            ++p;
            ++i;
            ++buf;
        }

        return node;
    }

    if (tk->kind == TK_ID) {
        Node *node = make_empty_node(ND_VAR, tk);
        Token *next = token_list_front(tokenList);
        return node;
    }

    if (match_char(tk, '(')) {
        Node *node = parse_expr(tokenList);
        expect_and_pop_char_punct(tokenList, ')');
        return node;
    }

    log_error_at_token("expect expression", tk);
    return NULL;
}

static Node *try_parse_variable_decl(TokenList *tokenList) {
    DataType const *dataType = parse_data_type(tokenList);
    if (!dataType) {
        return NULL;
    }

    Token *name = token_list_pop_front(tokenList);
    if (name->kind != TK_ID) {
        log_error_at_token(va("expect identifer, got \"%s\"", token_to_str(name)), name);
    }

    Node *varDecl = make_empty_node(ND_VAR_DECL, name);
    varDecl->dataType = dataType;

    if (match_char(token_list_front(tokenList), '=')) {
        token_list_pop_front(tokenList);  // pop "="

        if (match_char(token_list_front(tokenList), '{')) {
            token_list_pop_front(tokenList);

            varDecl->initializer = node_list_make();

            while (!match_char(token_list_front(tokenList), '}')) {
                if (varDecl->initializer->size) {
                    expect_and_pop_char_punct(tokenList, ',');
                }

                node_list_push_back(varDecl->initializer, parse_expr(tokenList));
            }

            expect_and_pop_char_punct(tokenList, '}');
        } else {
            varDecl->expr = parse_expr(tokenList);
        }
    }

    return varDecl;
}

// <compound-statement> ::= { {<declaration>}* {<statement>}* }
Node *parse_comp_stmt(TokenList *tokenList) {
    Node *compStmt = make_empty_node(ND_COMP_STMT, token_list_front(tokenList));
    compStmt->stmtList = node_list_make();

    expect_and_pop_char_punct(tokenList, '{');

    while (!match_char(token_list_front(tokenList), '}')) {
        Node *node = try_parse_variable_decl(tokenList);
        if (node) {
            expect_and_pop_char_punct(tokenList, ';');
            node_list_push_back(compStmt->stmtList, node);
        } else {
            node_list_push_back(compStmt->stmtList, parse_stmt(tokenList));
        }
    }

    expect_and_pop_char_punct(tokenList, '}');

    return compStmt;
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
Node *parse_stmt(TokenList *tokenList) {
    Token *tk = token_list_front(tokenList);

    // return statement
    if (tk->kind == TK_KEYWORD) {
        token_list_pop_front(tokenList);
        if (match_str(tk, "return")) {
            Node *node = make_empty_node(ND_RETURN, tk);
            node->expr = parse_expr(tokenList);
            expect_and_pop_char_punct(tokenList, ';');
            return node;
        }

        if (match_str(tk, "break")) {
            Node *node = make_empty_node(ND_BREAK, tk);
            expect_and_pop_char_punct(tokenList, ';');
            return node;
        }

        if (match_str(tk, "continue")) {
            Node *node = make_empty_node(ND_CONTINUE, tk);
            expect_and_pop_char_punct(tokenList, ';');
            return node;
        }

        if (match_str(tk, "if")) {
            Node *node = make_empty_node(ND_IF_ELSE, tk);
            expect_and_pop_char_punct(tokenList, '(');
            node->cond = parse_expr(tokenList);
            expect_and_pop_char_punct(tokenList, ')');
            node->ifStmt = parse_stmt(tokenList);
            if (match_str(token_list_front(tokenList), "else")) {
                token_list_pop_front(tokenList);  // pop "else"
                node->elseStmt = parse_stmt(tokenList);
            }
            return node;
        }

        if (match_str(tk, "while")) {
            Node *node = make_empty_node(ND_WHILE, tk);
            expect_and_pop_char_punct(tokenList, '(');
            node->cond = parse_expr(tokenList);
            expect_and_pop_char_punct(tokenList, ')');
            node->body = parse_stmt(tokenList);
            return node;
        }

        if (match_str(tk, "for")) {
            Node *node = make_empty_node(ND_FOR, tk);
            expect_and_pop_char_punct(tokenList, '(');
            // setup
            node->setup = try_parse_variable_decl(tokenList);
            expect_and_pop_char_punct(tokenList, ';');
            // condition
            if (!match_char(token_list_front(tokenList), ';')) {
                node->cond = parse_expr(tokenList);
            }
            expect_and_pop_char_punct(tokenList, ';');
            // increment
            node->increment = parse_expr_list(tokenList);
            expect_and_pop_char_punct(tokenList, ')');
            node->body = parse_stmt(tokenList);

            return node;
        }
    }

    // compound statement
    if (match_char(tk, '{')) {
        return parse_comp_stmt(tokenList);
    }

    // expression statement
    Node *exprStmt = make_empty_node(ND_EXPR_STMT, tk);
    if (match_char(tk, ';')) {
        token_list_pop_front(tokenList);
        exprStmt->expr = make_empty_node(ND_NULL_EXPR, tk);
    } else {
        exprStmt->expr = parse_expr(tokenList);
        expect_and_pop_char_punct(tokenList, ';');
    }

    return exprStmt;
}

// <function-definition> ::= {<declaration-specifier>}* <declarator> {<declaration>}* <compound-statement>
static Node *parse_obj(TokenList *tokenList) {
    Token *tk = token_list_front(tokenList);
    DataType const *type = parse_data_type(tokenList);
    if (!type) {
        log_error_at_token(va("expect type specifier, got %s", token_to_str(tk)), tk);
    }

    Token *identifier = token_list_pop_front(tokenList);
    if (identifier->kind != TK_ID) {
        log_error_at_token("expect identifer", identifier);
    }

    // no parameter for now
    expect_and_pop_char_punct(tokenList, '(');

    Node *func = make_empty_node(ND_FUNC_IMPL, identifier);
    func->dataType = type;
    func->argList = node_list_make();

    bool comma = false;
    int offset = -8;
    while (!match_char(token_list_front(tokenList), ')')) {
        if (comma) {
            expect_and_pop_char_punct(tokenList, ',');
        }
        comma = true;

        DataType const *dataType = parse_data_type(tokenList);
        if (!dataType) {
            log_error_at_token("expect type specifier", tk);
        }

        tk = token_list_pop_front(tokenList);
        if (tk->kind != TK_ID) {
            log_error_at_token("expect variable name", tk);
        }

        Node *arg = make_empty_node(ND_VAR_DECL, tk);
        arg->dataType = dataType;
        node_list_push_back(func->argList, arg);
    }

    token_list_pop_front(tokenList);  // pop ")"
    func->body = parse_comp_stmt(tokenList);

    return func;
}

Node *parse(TokenList *tokenList) {
    Node *root = make_empty_node(ND_TRANS_UNIT, token_list_front(tokenList));
    root->objList = node_list_make();

    while (tokenList->size) {
        node_list_push_back(root->objList, parse_obj(tokenList));
    }

    return root;
}

#define T Node *
#define T_NAME(x) Node##x
#define T_FUNC_NAME(x) node_##x
#include "containers/list_impl.h"
#undef T
#undef T_NAME
#undef T_FUNC_NAME
