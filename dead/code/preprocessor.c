#include "toyc.h"

/// TODO: refactor

static Token *copy_token(Token const *copy) {
    Token *t = CALLOC(sizeof(Token));
    t->kind = copy->kind;
    t->len = copy->len;
    t->loc.col = copy->loc.col;
    t->loc.fileCache = copy->loc.fileCache;
    t->loc.ln = copy->loc.ln;
    t->loc.p = copy->loc.p;
    return t;
}
/// macros
typedef struct
{
    Token const *name;
    TokenList *args;
    TokenList *body;
    bool isFunc;
} Macro;

// return -1 if not
static int is_token_macro_param(Macro const *macro, Token const *tk) {
    int i = 0;
    for (TokenListNode const *n = macro->args->begin; n; n = n->next, ++i) {
        Token const *param = n->data;
        if (tk->len == param->len && strncmp(tk->loc.p, param->loc.p, param->len) == 0) {
            return i;
        }
    }

    return -1;
}

static void debug_print_macro(Macro const *macro) {
    log("macro %s\n", token_to_str(macro->name));
    log("%d args:", macro->args->size);
    for (TokenListNode const *n = macro->args->begin; n; n = n->next) {
        log(" %s,", token_to_str(n->data));
    }
    log("\n");
    log("%d body:", macro->body->size);
    for (TokenListNode const *n = macro->body->begin; n; n = n->next) {
        log(" %s,", token_to_str(n->data));
    }
    log("\n");
}

#define T Macro *
#define T_NAME(x) Macro##x
#define T_FUNC_NAME(x) macro_##x
#include "containers/dict_decl.h"
#include "containers/dict_impl.h"
#undef T
#undef T_NAME
#undef T_FUNC_NAME

static MacroDict *s_macros = NULL;
static IntDict *s_headers = NULL;

extern expect_and_pop_char_punct(TokenList *tokenList, char c);

static TokenList *parse_preproc_args(TokenList *input) {
    TokenList *args = token_list_make();

    expect_and_pop_char_punct(input, '(');

    bool firstParam = true;
    while (!match_char(token_list_front(input), ')')) {
        if (firstParam) {
            Token *tk = token_list_pop_front(input);
            assertfmt(tk->kind == TK_ID, "expect identifier, got '%s'", token_to_str(tk));
            token_list_push_back(args, tk);
            firstParam = false;
            continue;
        }

        expect_and_pop_char_punct(input, ',');

        Token *tk = token_list_pop_front(input);
        assertfmt(tk->kind == TK_ID, "expect identifier, got '%s'", token_to_str(tk));
        token_list_push_back(args, tk);
    }

    token_list_pop_front(input);  // pop ")"
    return args;
}

static TokenList *parse_preproc_call_arg(TokenList *input) {
    TokenList *args = token_list_make();
    Token *tk = token_list_front(input);
    int brackets = 1;
    while (!match_char(tk, ',')) {
        if (match_char(tk, '(')) {
            ++brackets;
        } else if (match_char(tk, ')')) {
            if (--brackets == 0) {
                break;
            }
        }

        token_list_push_back(args, token_list_pop_front(input));
        tk = token_list_front(input);
    }
    return args;
}

TokenList *preprocess(TokenList *input) {
    /// TODO: refactor
    if (!s_headers) {
        s_headers = int_dict_make();
        int_dict_insert(s_headers, "stdio.h", 1);
    }

    if (!s_macros) {
        s_macros = macro_dict_make();
    }

    TokenList *processed = token_list_make();

    while (input->size) {
        Token *tk = token_list_pop_front(input);
        int ln = tk->loc.ln;
        if (match_char(tk, '#')) {
            Token *directive = token_list_pop_front(input);
            if (match_str(directive, "include")) {
                tk = token_list_front(input);
                if (tk->kind != TK_STRING) {
                    if (ln != directive->loc.ln) {
                        Loc loc = {.col = directive->loc.col + directive->len, .ln = ln, .fileCache = directive->loc.fileCache};
                        log_error_at_loc("#include expects \"FILENAME\" or <FILENAME>", &loc, 1);
                    } else {
                        log_error_at_token("#include expects \"FILENAME\" or <FILENAME>", tk);
                    }
                } else {
                    if (int_dict_nhas(s_headers, tk->loc.p, tk->len))  // skip quotes
                    {
                        token_list_pop_front(input);
                    } else {
                        assertfmt(0, "local file include not supported");
                    }
                }
            } else if (match_str(directive, "define")) {
                tk = token_list_pop_front(input);
                assertfmt(tk->kind == TK_ID, "expect identifier, got '%s'", token_to_str(tk));
                assert(tk->loc.ln == ln);

                Macro *macro = CALLOC(sizeof(Macro));
                macro->name = tk;

                if (match_char(token_list_front(input), '(')) {
                    macro->isFunc = true;
                    macro->body = token_list_make();
                    macro->args = parse_preproc_args(input);

                    tk = token_list_front(input);
                    while (tk->loc.ln == ln) {
                        /// TODO: handle '\'
                        token_list_push_back(macro->body, token_list_pop_front(input));
                        tk = token_list_front(input);
                    }

                    // debug_print_macro(macro);
                    macro_dict_ninsert(s_macros, macro->name->loc.p, macro->name->len, macro);
                } else {
                    log_unexpected_token(tk);
                }
            } else {
                log_error_at_token(
                    va("invalid preprocessing directive #%s", token_to_str(directive)),
                    directive);
            }
        } else if (tk->kind == TK_ID && macro_dict_nhas(s_macros, tk->loc.p, tk->len)) {
            Macro *inMacro = NULL;
            macro_dict_nget(s_macros, tk->loc.p, tk->len, &inMacro);
            Macro const *macro = inMacro;
            if (macro->isFunc) {
                TokenList *argv[16];
                const int argc = macro->args->size;
                assert(argc < ARRAY_SIZE(argv));
                expect_and_pop_char_punct(input, '(');

                bool isFirstArg = true;
                int count = 0;
                Token *tk = token_list_front(input);

                while (!match_char(tk, ')')) {
                    argv[count++] = parse_preproc_call_arg(input);

                    tk = token_list_pop_front(input);  // pop "," or ")"
                }

                assertfmt(count == argc, "arg count mismatch");
                /// TODO: replace

                TokenList *generated = token_list_make();

                for (TokenListNode const *n = macro->body->begin; n; n = n->next) {
                    Token *tk = n->data;
                    if (tk->kind == TK_ID) {
                        // check if parameter
                        int index = is_token_macro_param(macro, tk);
                        if (index != -1) {
                            assert(index < count);
                            TokenList *expansion = argv[index];

                            if (match_char(n->prev->data, '#')) {
                                Token *st = CALLOC(sizeof(Token));
                                st->loc.p = expansion->begin->data->loc.p;
                                st->loc.fileCache = expansion->begin->data->loc.fileCache;
                                st->loc.ln = expansion->begin->data->loc.ln;
                                st->loc.col = expansion->begin->data->loc.col;
                                st->kind = TK_STRING;
                                st->len = (int)(expansion->end->data->loc.p - st->loc.p + expansion->end->data->len);
                                token_list_push_back(generated, st);
                                continue;
                            }

                            for (TokenListNode const *e = expansion->begin; e; e = e->next) {
                                token_list_push_back(generated, copy_token(e->data));
                            }
                            continue;
                        }
                    }

                    if (!match_char(tk, '#')) {
                        token_list_push_back(generated, copy_token(tk));
                    }
                }

                for (TokenListNode *n = generated->begin; n; n = n->next) {
                    token_list_push_back(processed, n->data);
                }

                token_list_destroy(&generated);
            } else {
                assertfmt(0, "macro variable not handled");
            }
        } else {
            token_list_push_back(processed, tk);
        }
    }

    return processed;
}