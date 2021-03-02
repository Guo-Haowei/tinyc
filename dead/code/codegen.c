#include "toyc.h"

static FILE *s_file = NULL;

#define writeln(...)              \
    fprintf(s_file, __VA_ARGS__); \
    fputc('\n', s_file)

#define write(...) fprintf(s_file, __VA_ARGS__);

static NodeList *s_stringLiteralCache = NULL;

static void gen(Node const *node);
static void gen_unary_internal(Node const *node);
static void gen_binary_internal(Node const *node);

static void gen_addr(const Node *node);

static void gen_subscript_addr(const Node *node) {
    assert(node);
    assert(node->kind == ND_SUBSCRIPT);
    assert(node->lhs && node->rhs);
    //assert(node->lhs->dataType->base);

    gen(node->rhs);
    /// FIXME: assume 4 for now
    //writeln("  imul eax, %d", data_type_size(node->dataType->base));
    writeln("  imul eax, %d", 4);
    writeln("  push ecx");
    writeln("  mov ecx, eax");
    gen_addr(node->lhs);
    writeln("  add eax, ecx");
    writeln("  pop ecx");
}

static void gen_addr(const Node *node) {
    if (node->kind == ND_VAR) {
        /// NOTE: only if local variable
        writeln("  mov eax, ebp");
        writeln("  sub eax, %d", node->offset);
    } else if (node->kind == ND_SUBSCRIPT) {
        gen_subscript_addr(node);
    } else if (node->kind == ND_DEREF) {
        gen(node->rhs);
    } else {
        assertfmt(0, "expect a rvalue");
    }
}

void gen_unary_internal(Node const *node) {
    assert(node);

    /// TODO: refactor this
    if (node->kind == ND_I_INC || node->kind == ND_I_DEC) {
        assert(node->lhs);
        gen_addr(node->lhs);
        writeln("  push ecx");
        writeln("  mov ecx, eax");
        writeln("  mov eax, [ecx]");
        /// FIXME: change when type is a pointer, should inc by 4
        writeln("  push eax");
        writeln("  %s eax, %d", node->kind == ND_I_INC ? "add" : "sub", 1);
        writeln("  mov [ecx], eax");
        writeln("  pop eax");
        writeln("  pop ecx");
        return;
    }

    assert(!node->lhs);
    assert(node->rhs);

    writeln("  ; unary operator %s", token_to_str(node->token));
    gen(node->rhs);

    if (node->kind == ND_NOT) {
        writeln("  mov ecx, eax");   // save eax
        writeln("  xor eax, eax");   // zero out eax
        writeln("  test ecx, ecx");  // set ZF(zero flag)
        writeln("  sete al");        // set eax to ZF
        return;
    }

    if (node->kind == ND_NEG) {
        writeln("  neg eax");
        return;
    }

    if (node->kind == ND_POS) {
        return;
    }

    if (node->kind == ND_INC_I || node->kind == ND_DEC_I) {
        assert(node->rhs);
        gen_addr(node->rhs);
        writeln("  push ecx");
        writeln("  mov ecx, eax");
        writeln("  mov eax, [ecx]");
        /// FIXME: change when type is a pointer, should inc by 4
        writeln("  %s eax, %d", node->kind == ND_INC_I ? "add" : "sub", 1);
        writeln("  mov [ecx], eax");
        writeln("  pop ecx");
        return;
    }

    if (node->kind == ND_ADDRESS) {
        assert(node->rhs && node->rhs->kind == ND_VAR);
        writeln("  mov eax, ebp");
        writeln("  sub eax, %d", node->rhs->offset);
        return;
    }

    if (node->kind == ND_DEREF) {
        /// TODO: move these asserts to validation pass
        assert(node->rhs);
        gen(node->rhs);
        // writeln("  push ecx");
        // writeln("  mov ecx, eax", node->rhs->offset);
        writeln("  mov eax, [eax]");
        // writeln("  pop ecx");
        return;
    }

    assertfmt(0, "Unexpected unary operator %s", token_to_str(node->token));
    return;
}

void gen_binary_internal(Node const *node) {
    assert(node);
    assert(node->lhs);
    assert(node->rhs);

    writeln("  ; binary operator %.*s", node->token->len, node->token->loc.p);
    if (node->kind == ND_AND || node->kind == ND_OR) {
        gen(node->lhs);
        writeln("  cmp eax, 0");
        writeln("  %s LABEL_LOGICAL_%d", node->kind == ND_AND ? "je" : "jne", node->guid);
        gen(node->rhs);
        writeln("LABEL_LOGICAL_%d:", node->guid);
        writeln("  mov ecx, eax");   // save eax
        writeln("  xor eax, eax");   // zero out eax
        writeln("  test ecx, ecx");  // set ZF(zero flag)
        writeln("  setne al");       // set eax to not ZF
        return;
    }

    gen(node->rhs);
    writeln("  push eax");
    gen(node->lhs);
    writeln("  pop ecx");

    if (node->kind == ND_ADD || node->kind == ND_SUB || node->kind == ND_MUL) {
        char const *op = node->kind == ND_ADD ? "add" : node->kind == ND_SUB ? "sub"
                                                                             : "imul";
        writeln("  %s eax, ecx", op);
        return;
    }

    if (node->kind == ND_DIV || node->kind == ND_REM) {
        writeln("  cdq");
        writeln("  idiv ecx");
        if (node->kind == ND_REM) {
            writeln("  mov eax, edx");
        }
        return;
    }

    writeln("  cmp eax, ecx");
    writeln("  mov eax, 0");
    char const *op = node->kind == ND_EQ ? "sete" : node->kind == ND_NE ? "setne"
                                                : node->kind == ND_LT   ? "setl"
                                                : node->kind == ND_GT   ? "setg"
                                                : node->kind == ND_LE   ? "setle"
                                                : node->kind == ND_GE   ? "setge"
                                                                        : NULL;
    assertfmt(op, "Unexpected binary operator %s", token_to_str(node->token));
    writeln("  %s al", op);
    return;
}

void gen(Node const *node) {
    assert(node);
    if (node->binary) {
        gen_binary_internal(node);
        return;
    }

    if (node->unary) {
        gen_unary_internal(node);
        return;
    }

    switch (node->kind) {
        case ND_NULL_EXPR:
            break;
        case ND_NUMBER:
            writeln("  mov eax, %d", node->intVal);
            break;
        case ND_STRING:
            writeln("  mov eax, _str_literal_%d", node->guid);
            assert(s_stringLiteralCache);
            node_list_push_back(s_stringLiteralCache, node);
            break;
        case ND_RETURN:
            gen(node->expr);
            writeln("  jmp _%s%d_exit_point", token_to_str(node->owner->token), node->owner->guid);
            break;
        case ND_FUNC_CALL:
            Token const *funcToken = node->identifier->token;
            writeln("  ; call to %s", token_to_str(funcToken));
            for (NodeListNode *it = node->argList->end; it; it = it->prev) {
                gen(it->data);
                writeln("  push eax");
            }
            writeln("  call _%s", token_to_str(funcToken));
            writeln("  add esp, %d", 4 * node->argList->size);
            break;
        case ND_EXPR_STMT:
            gen(node->expr);
            break;
        case ND_COMP_STMT:
            for (NodeListNode *it = node->stmtList->begin; it; it = it->next) {
                gen(it->data);
            }
            if (node->scopedMemory) {
                writeln("  add esp, %d", node->scopedMemory);
            }
            break;
        case ND_TERNARY:
            writeln("  ; ternary");
            gen(node->cond);
            writeln("  cmp eax, 0");
            writeln("  je TERNARY_RHS_%d", node->guid);
            gen(node->lhs);
            writeln("  jmp TERNARY_RHS_SKIP_%d", node->guid);
            writeln("TERNARY_RHS_%d:", node->guid);
            gen(node->rhs);
            writeln("TERNARY_RHS_SKIP_%d:", node->guid);
            break;
        case ND_IF_ELSE:
            writeln("  ; if else");
            gen(node->cond);
            writeln("  cmp eax, 0");
            writeln("  je BRANCH_SKIP_IF_%d", node->guid);
            gen(node->ifStmt);
            if (node->elseStmt) {
                writeln("  jmp BRANCH_SKIP_ELSE_%d", node->guid);
            }
            writeln("BRANCH_SKIP_IF_%d:", node->guid);
            if (node->elseStmt) {
                gen(node->elseStmt);
                writeln("BRANCH_SKIP_ELSE_%d:", node->guid);
            }
            break;
        case ND_CONTINUE:
            writeln("  jmp LOOP_CONTINUE_%d", node->owner->guid);
            break;
        case ND_BREAK:
            writeln("  jmp LOOP_BREAK_%d", node->owner->guid);
            break;
        case ND_WHILE:
            writeln("  ; while");
            writeln("LOOP_CONTINUE_%d:", node->guid);
            gen(node->cond);
            writeln("  cmp eax, 0");
            writeln("  je LOOP_BREAK_%d", node->guid);
            gen(node->body);
            writeln("  jmp LOOP_CONTINUE_%d", node->guid);
            writeln("LOOP_BREAK_%d:", node->guid);
            break;
        case ND_FOR:
            writeln("  ; for");
            if (node->setup) {
                gen(node->setup);
            }
            writeln("LOOP_BEGIN_%d:", node->guid);
            if (node->cond) {
                gen(node->cond);
                writeln("  cmp eax, 0");
                writeln("  je LOOP_BREAK_%d", node->guid);
            }
            gen(node->body);
            writeln("LOOP_CONTINUE_%d:", node->guid);
            for (NodeListNode *n = node->increment->begin; n; n = n->next) {
                gen(n->data);
            }
            writeln("  jmp LOOP_BEGIN_%d", node->guid);
            writeln("LOOP_BREAK_%d:", node->guid);
            if (node->scopedMemory) {
                writeln("  add esp, %d", node->scopedMemory);
            }
            break;
        case ND_VAR:
            /// TODO: refactor this
            // if (node->dataType->kind == DT_ARRAY) {
            //     writeln("  mov eax, ebp");
            //     writeln("  sub eax, %d", node->offset);
            // // } else {
            writeln("  mov eax, [ebp - %d]", node->offset);
            // }
            break;
        case ND_ASSIGN:
            writeln("  ; assignment");
            gen(node->rhs);
            writeln("  push ecx");
            writeln("  mov ecx, eax");
            gen_addr(node->lhs);
            writeln("  mov [eax], ecx");
            writeln("  mov eax, ecx");
            writeln("  pop ecx");
            break;
        case ND_VAR_DECL:
            writeln("  sub esp, %d", data_type_size(node->dataType));
            if (node->expr) {
                gen(node->expr);
                writeln("  mov [esp], eax");
            } else if (node->initializer) {
                int offset = 0;
                for (NodeListNode *it = node->initializer->begin; it; it = it->next) {
                    gen(it->data);
                    writeln("  mov [esp + %d], eax", offset);
                    offset += 4;
                }
            }
            break;
        case ND_FUNC_IMPL:
            writeln("_%s:", token_to_str(node->token));
            writeln("  push ebp");
            writeln("  mov ebp, esp");
            gen(node->body);
            writeln("_%s%d_exit_point:", token_to_str(node->token), node->guid);
            writeln("  mov esp, ebp");
            writeln("  pop ebp");
            writeln("  ret");
            break;
        case ND_TRANS_UNIT:
            for (NodeListNode *n = node->objList->begin; n; n = n->next) {
                gen(n->data);
            }
            break;
        case ND_SUBSCRIPT:
            gen_subscript_addr(node);
            writeln("  mov eax, [eax]");
            break;
        default:
            assertfmt(0, "Unhandled node type %s", eNode_to_str(node->kind));
            break;
    }
}

void codegen(const Node *node, FILE *fp) {
    if (!s_stringLiteralCache) {
        s_stringLiteralCache = node_list_make();
    }

    s_file = fp;

    writeln("  section .text");
    writeln("  global _main");
    writeln("  extern _printf");

    gen(node);

    writeln("  section .data");
    /// TODO: refactor this
    for (NodeListNode *it = s_stringLiteralCache->begin; it; it = it->next) {
        writeln("_str_literal_%d: ; %s", it->data->guid, token_to_str(it->data->token));
        write("  db ");
        for (const char *s = it->data->strVal; *s; ++s) {
            write("%d,", *s);
        }
        writeln("0");
    }

    if (s_stringLiteralCache) {
        node_list_destroy(&s_stringLiteralCache);
    }
}
