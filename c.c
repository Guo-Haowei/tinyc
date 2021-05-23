enum { _TkOffset = 128,
       CInt, Id, CStr, CChar,
       TkNeq, TkEq, TkGe, TkLe,
       TkAddTo, TkSubFrom, TkInc, TkDec, TkAnd, TkOr, LShift, RShift,
       _KeywordStart, Int, Char, Void,
       Break, Cont, Else, Enum, If, Return, While,
       Printf, Fopen, Fgetc, Malloc, Memset, Exit,
       _KeywordEnd,
       Add, Sub, Mul, Div, Rem,
       Mov, Push, Pop, Load, Save,
       Neq, Eq, Gt, Ge, Lt, Le, And, Or,
       Not, Ret, Jz, Jnz, Jump, Call,
       _BreakStub, _ContStub };
enum { Undefined, Global, Param, Local, Func, Const };
enum { EAX = 1, EBX, ECX, EDX, ESP, EBP, IMME };
enum { Kind, Value, Ln, Start, End, TokenSize };
enum { TkIdx, Scope, DType, Storage, Address, SymSize };
enum { OpCode, Imme, OpSize };
enum { InsIdx = 1, CallSize };
char *g_ram, *g_src;
int g_reserved, g_bss,
    *g_tks, g_tkCnt, g_tkIter,
    *g_syms, g_symCnt,
    *g_ops, g_opCnt,
    *g_regs,
    g_entry,
    g_scopeId, *g_scopes, g_scopeCnt,
    *g_calls, g_callCnt;
void panic(char* fmt) {
    printf("[panic] %s\n", fmt);
    exit(1);
}
int streq(char* p1, char* p2, int len) {
    while (len--) {
        if (*p1 == 0 || *p2 == 0) return 1;
        if (*p1 != *p2) return 0;
        p1 += 1; p2 += 1;
    }
    return 1;
}
int strlen(char* p) {
    int len = 0;
    while (*p++) { len += 1; }
    return len;
}
void lex() {
    int ln = 1, range = _KeywordEnd - _KeywordStart - 1;
    char *p = g_src;
    while (*p) {
        if (*p == '#' || (*p == '/' && *(p + 1) == '/')) { while (*p && *p != 10) ++p; }
        else if ((*p == ' ' || *p == 9 || *p == 10 || *p == 13)) { ln += (*p == 10); ++p; }
        else {
            g_tks[((g_tkCnt) * TokenSize) + Ln] = ln;
            g_tks[((g_tkCnt) * TokenSize) + Start] = p;
            if (((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')) || *p == '_') {
                g_tks[((g_tkCnt) * TokenSize) + Kind] = Id;
                ++p; while (((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')) || (*p >= '0' && *p <= '9') || *p == '_') { ++p; }
                g_tks[((g_tkCnt) * TokenSize) + End] = p;
                char *keywords = "int\0     char\0    void\0    break\0   continue\0"
                                 "else\0    enum\0    if\0      return\0  while\0   "
                                 "printf\0  fopen\0   fgetc\0   malloc\0  memset\0  "
                                 "exit\0    ";
                int i = 0, start = g_tks[((g_tkCnt) * TokenSize) + Start], len = g_tks[((g_tkCnt) * TokenSize) + End] - start;
                while (i < range) {
                    char* kw = keywords + (i * 9);
                    int kwlen = strlen(kw);
                    if (kwlen == len && streq(start, kw, 8)) {
                        g_tks[((g_tkCnt) * TokenSize) + Kind] = _KeywordStart + i + 1;
                        break;
                    }
                    ++i;
                }
                g_tkCnt += 1;
            } else if (*p == '0' && p[1] == 'x') {
                g_tks[((g_tkCnt) * TokenSize) + Kind] = CInt;
                int result = 0;
                p += 2; while(((*p >= '0' && *p <= '9') || (*p >= 'A' && *p <= 'F'))) {
                    result = (result << 4) + ((*p < 'A') ? (*p - '0') : (*p - 55));
                    ++p;
                }
                g_tks[((g_tkCnt) * TokenSize) + Value] = result;
                g_tks[((g_tkCnt++) * TokenSize) + End] = p;
            } else if ((*p >= '0' && *p <= '9')) {
                g_tks[((g_tkCnt) * TokenSize) + Kind] = CInt;
                int result = 0;
                while ((*p >= '0' && *p <= '9')) { result = result * 10 + (*p - '0'); ++p; }
                g_tks[((g_tkCnt) * TokenSize) + Value] = result;
                g_tks[((g_tkCnt++) * TokenSize) + End] = p;
            } else if (*p == '"') {
                g_tks[((g_tkCnt) * TokenSize) + Kind] = CStr;
                ++p; while (*p != '"') { ++p; };
                g_tks[((g_tkCnt++) * TokenSize) + End] = ++p;
            } else if (*p == 39) {
                g_tks[((g_tkCnt) * TokenSize) + Kind] = CChar;
                g_tks[((g_tkCnt) * TokenSize) + Value] = p[1];
                g_tks[((g_tkCnt++) * TokenSize) + End] = (p += 3);
            } else {
                g_tks[((g_tkCnt) * TokenSize) + Kind] = *p;
                if ((*p == '=' && p[1] == '=')) { g_tks[((g_tkCnt) * TokenSize) + Kind] = TkEq; ++p; }
                else if ((*p == '!' && p[1] == '=')) { g_tks[((g_tkCnt) * TokenSize) + Kind] = TkNeq; ++p; }
                else if ((*p == '&' && p[1] == '&')) { g_tks[((g_tkCnt) * TokenSize) + Kind] = TkAnd; ++p; }
                else if ((*p == '|' && p[1] == '|')) { g_tks[((g_tkCnt) * TokenSize) + Kind] = TkOr; ++p; }
                else if (*p == '+') {
                    if (p[1] == '+') { g_tks[((g_tkCnt) * TokenSize) + Kind] = TkInc; ++p; }
                    else if (p[1] == '=') { g_tks[((g_tkCnt) * TokenSize) + Kind] = TkAddTo; ++p; }
                } else if (*p == '-') {
                    if (p[1] == '-') { g_tks[((g_tkCnt) * TokenSize) + Kind] = TkDec; ++p; }
                    else if (p[1] == '=') { g_tks[((g_tkCnt) * TokenSize) + Kind] = TkSubFrom; ++p; }
                } else if (*p == '>') {
                    if (p[1] == '=') { g_tks[((g_tkCnt) * TokenSize) + Kind] = TkGe; ++p; }
                    else if (p[1] == '>') { g_tks[((g_tkCnt) * TokenSize) + Kind] = RShift; ++p; }
                } else if (*p == '<') {
                    if (p[1] == '=') { g_tks[((g_tkCnt) * TokenSize) + Kind] = TkLe; ++p; }
                    else if (p[1] == '<') { g_tks[((g_tkCnt) * TokenSize) + Kind] = LShift; ++p; }
                }
                g_tks[((g_tkCnt++) * TokenSize) + End] = ++p;
            }
        }
    }
    return;
}
void dump_tokens() {
    printf("-------- lex --------\n");
    int indent = 0, i = 0, ln = 0;
    while (i < g_tkCnt) {
        int tkln = g_tks[((i) * TokenSize) + Ln];
        int kind = g_tks[((i) * TokenSize) + Kind];
        int start = g_tks[((i) * TokenSize) + Start];
        int end = g_tks[((i) * TokenSize) + End];
        int len = end - start;
        if (kind == '{') { indent += 1; }
        else if (kind == '}') { indent -= 1; }
        if (ln != tkln) {
            printf("\n%-3d:%.*s", tkln, indent * 4, "                                        ");
            ln = tkln;
        }
        char* names = "Int   Char  Void  Break Cont  Else  Enum  If    "
                      "Ret   While Print Fopen Fgetc MallocMemsetExit  ";
        printf("%.*s", len, start);
        if (kind >= Int) {
            printf("{");
            char *p = names + 6 * (kind - Int); int ii = 0;
            while (ii < 6) {
                if (*p == ' ') break;
                printf("%c", *p);
                ++ii; ++p;
            }
            printf("}");
        }
        printf(" ");
        ++i;
    }
    printf("\n");
    return;
}
void enter_scope() {
    if (g_scopeCnt >= 128) {
        panic("scope overflow");
    }
    g_scopes[g_scopeCnt++] = ++g_scopeId;
    return;
}
void exit_scope() {
    if (g_scopeCnt <= 0) {
        panic("scope overflow");
    }
    int i = g_symCnt - 1;
    while (g_syms[((i) * SymSize) + Scope] == g_scopes[g_scopeCnt - 1]) {
        --g_symCnt, --i;
    }
    --g_scopeCnt;
    return;
}
int expect(int kind) {
    if (g_tks[((g_tkIter) * TokenSize) + Kind] != kind) {
        int start = g_tks[((g_tkIter) * TokenSize) + Start], end = g_tks[((g_tkIter) * TokenSize) + End];
        { printf("error:%d: expected token '%c'(%d), got '%.*s'\n", g_tks[((g_tkIter) * TokenSize) + Ln], kind < 128 ? kind : ' ', kind, end - start, start); exit(1); }
                                                                                       ;
    }
    return g_tkIter++;
}
int expect_type() {
    int base_type = g_tks[((g_tkIter) * TokenSize) + Kind];
    if ((base_type >= Int && base_type <= Void)) {
        ++g_tkIter;
        int ptr = 0;
        while (g_tks[((g_tkIter) * TokenSize) + Kind] == '*') { ptr = (ptr << 8) | 0xFF; ++g_tkIter; }
        return (ptr << 16) | base_type;
    }
    int start = g_tks[((g_tkIter) * TokenSize) + Start], end = g_tks[((g_tkIter) * TokenSize) + End];
    { printf("error:%d: expected type specifier, got '%.*s'\n", g_tks[((g_tkIter) * TokenSize) + Ln], end - start, start); exit(1); };
}
void instruction(int op, int imme) {
    g_ops[((g_opCnt) * OpSize) + OpCode] = op;
    g_ops[((g_opCnt) * OpSize) + Imme] = imme;
    ++g_opCnt;
    return;
}
int primary_expr() {
    int tkIdx = g_tkIter++;
    char* start = g_tks[((tkIdx) * TokenSize) + Start];
    char* end = g_tks[((tkIdx) * TokenSize) + End];
    int ln = g_tks[((tkIdx) * TokenSize) + Ln];
    int kind = g_tks[((tkIdx) * TokenSize) + Kind];
    int value = g_tks[((tkIdx) * TokenSize) + Value];
    int len = end - start;
    if (kind == CInt || kind == CChar) {
        instruction(Mov | (EAX << 8) | (IMME << 24), value);
        return Int;
    }
    if (kind == CStr) {
        instruction(Mov | (EAX << 8) | (IMME << 24), g_bss);
        while (1) {
            len = len - 1;
            int i = 1;
            while (i < len) {
                int c = start[i];
                if (c == 92) {
                    c = start[i += 1];
                    if (c == 'n') { c = 10; }
                    else if (c == '0') { c = 0; }
                    else { { printf("error:%d: unknown escape sequence '%c'\n", ln, c); exit(1); }; }
                }
                *((char*)g_bss++) = c;
                ++i;
            }
            if (g_tks[((g_tkIter) * TokenSize) + Kind] != CStr) break;
            start = g_tks[((g_tkIter) * TokenSize) + Start];
            end = g_tks[((g_tkIter) * TokenSize) + End];
            ln = g_tks[((g_tkIter) * TokenSize) + Ln];
            len = end - start;
            ++g_tkIter;
        }
        *((char*)g_bss++) = 0;
        g_bss = ((g_bss + 3) & -4);
        return (0xFF0000 | Char);
    }
    if (kind == '(') {
        int data_type = expr();
        expect(')');
        return data_type;
    }
    if (kind == Id) {
        if (g_tks[((g_tkIter) * TokenSize) + Kind] == '(') {
            ++g_tkIter;
            int argc = 0;
            while (g_tks[((g_tkIter) * TokenSize) + Kind] != ')') {
                if (argc > 0) expect(',');
                assign_expr();
                instruction(Push | (EAX << 24), 0);
                ++argc;
            }
            g_calls[((g_callCnt) * CallSize) + TkIdx] = tkIdx;
            g_calls[((g_callCnt++) * CallSize) + InsIdx] = g_opCnt;
            instruction(Call, 0);;
            if (argc) { instruction(Add | (ESP << 8) | (ESP << 16) | (IMME << 24), (argc << 2)); }
            expect(')');
            return Int;
        }
        int address = 0, type = Undefined, data_type = 0, i = g_symCnt - 1;
        while (i >= 0) {
            int tmp = g_syms[((i) * SymSize) + TkIdx];
            char *tmpstart = g_tks[((tmp) * TokenSize) + Start], *tmpend = g_tks[((tmp) * TokenSize) + End];
            if (len == (tmpend - tmpstart) && streq(start, tmpstart, len)) {
                address = g_syms[((i) * SymSize) + Address];
                type = g_syms[((i) * SymSize) + Storage];
                data_type = g_syms[((i) * SymSize) + DType];
                break;
            }
            --i;
        }
        if (type == Global) {
            instruction(Mov | (EDX << 8) | (IMME << 24), address);
            instruction(Load | (EAX << 8) | (EDX << 16), 4);
            return data_type;
        }
        if (type == Const) {
            instruction(Mov | (EAX << 8) | (IMME << 24), address);
            return Int;
        }
        if (type == Undefined) {
            { printf("error:%d: '%.*s' undeclared\n", ln, len, start); exit(1); };
        }
        instruction(Sub | (EDX << 8) | (EBP << 16) | (IMME << 24), (address));
        instruction(Load | (EAX << 8) | (EDX << 16), 4);
        return data_type;
    }
    if (kind == Printf) {
        expect('(');
        int argc = 0;
        while (g_tks[((g_tkIter) * TokenSize) + Kind] != ')') {
            if (argc > 0) expect(',');
            assign_expr();
            instruction(Push | (EAX << 24), 0);
            ++argc;
        }
        if (argc > 8) panic("printf supports at most %d args");
        instruction(Sub | (ESP << 8) | (ESP << 16) | (IMME << 24), ((8 - argc) << 2));
        instruction(Printf, argc);
        instruction(Add | (ESP << 8) | (ESP << 16) | (IMME << 24), (8 << 2));
        expect(')');
        return Int;
    }
    int paramCnt = 0, ret = Void, i = 0;
    if (kind == Fopen) { paramCnt = 2; ret = (0xFF0000 | Void); }
    else if (kind == Fgetc) { paramCnt = 1; ret = Int; }
    else if (kind == Malloc) { paramCnt = 1; ret = (0xFF0000 | Void); }
    else if (kind == Exit) { paramCnt = 1; ret = Void; }
    else { { printf("error:%d: expected expression, got '%.*s'\n", ln, len, start); exit(1); }; }
    expect('(');
    while (i < paramCnt) {
        if (i++ > 0) { expect(','); }
        assign_expr();
        instruction(Push | (EAX << 24), 0);
    }
    instruction(kind, 0);
    instruction(Add | (ESP << 8) | (ESP << 16) | (IMME << 24), (4 * paramCnt));
    expect(')');
    return ret;
}
int post_expr() {
    int data_type = primary_expr();
    while (1) {
        int kind = g_tks[((g_tkIter) * TokenSize) + Kind];
        int ln = g_tks[((g_tkIter) * TokenSize) + Ln];
        if (kind == '[') {
            ++g_tkIter;
            if (!(0xFF0000 & data_type)) {
                { printf("error:%d: attempted to dereference a non-pointer type 0x%X\n", ln, data_type); exit(1); };
            }
            instruction(Push | (EAX << 24), 0);
            assign_expr();
            int is_charptr = data_type == (0xFF0000 | Char);
            if (!is_charptr) {
                instruction(Mul | (EAX << 8) | (EAX << 16) | (IMME << 24), (4));
            }
            instruction(Pop | (EBX << 8), 0);
            instruction(Add | (EAX << 8) | (EBX << 16) | (EAX << 24), (0));
            instruction(Mov | (EDX << 8) | (EAX << 24), 0);
            if (is_charptr) { instruction(Load | (EAX << 8) | (EAX << 16), 1); }
            else { instruction(Load | (EAX << 8) | (EAX << 16), 4); }
            expect(']');
            data_type = ((data_type >> 8) & 0xFF0000) | ((data_type & 0xFFFF));
        } else if (kind == TkInc || kind == TkDec) {
            ++g_tkIter;
            instruction(Load | (EAX << 8) | (EDX << 16), 4);
            instruction(Mov | (EBX << 8) | (EAX << 24), 0);
            int value = ((0xFF0000 & data_type) && data_type != (0xFF0000 | Char)) ? 4 : 1;
            int op = kind == TkInc ? Add : Sub;
            instruction(((op) | (EBX << 8) | (EBX << 16) | (IMME << 24)), value);
            instruction(((Save) | (EDX << 8) | (EBX << 16) | (0 << 24)), 4);
        } else {
            break;
        }
    }
    return data_type;
}
int unary_expr() {
    int kind = g_tks[((g_tkIter) * TokenSize) + Kind];
    int ln = g_tks[((g_tkIter) * TokenSize) + Ln];
    if (kind == '!') {
        ++g_tkIter;
        int data_type = unary_expr();
        instruction(((Not) | (EAX << 8) | (0 << 16) | (0 << 24)), 0);
        return data_type;
    }
    if (kind == '+') {
        ++g_tkIter;
        return unary_expr();
    }
    if (kind == '-') {
        ++g_tkIter;
        int data_type = unary_expr();
        instruction(Mov | (EBX << 8) | (IMME << 24), 0);
        instruction(Sub | (EAX << 8) | (EBX << 16) | (EAX << 24), (0));
        return data_type;
    }
    if (kind == '*') {
        ++g_tkIter;
        int data_type = unary_expr();
        if (!(0xFF0000 & data_type)) {
            { printf("error:%d: attempted to dereference a non-pointer type 0x%X\n", ln, data_type); exit(1); };
        }
        instruction(Mov | (EDX << 8) | (EAX << 24), 0);
        if (data_type == (0xFF0000 | Char)) instruction(Load | (EAX << 8) | (EDX << 16), 1);
        else instruction(Load | (EAX << 8) | (EDX << 16), 4);
        return ((data_type >> 8) & 0xFF0000) | (0xFFFF & data_type);
    }
    if (kind == TkInc || kind == TkDec) {
        ++g_tkIter;
        int data_type = unary_expr();
        instruction(Load | (EAX << 8) | (EDX << 16), 4);
        int value = ((0xFF0000 & data_type) && data_type != (0xFF0000 | Char)) ? 4 : 1;
        int op = kind == TkInc ? Add : Sub;
        instruction(((op) | (EAX << 8) | (EAX << 16) | (IMME << 24)), value);
        instruction(((Save) | (EDX << 8) | (EAX << 16) | (0 << 24)), 4);
        return data_type;
    }
    return post_expr();
}
int cast_expr() {
    if (g_tks[((g_tkIter) * TokenSize) + Kind] == '(') {
        int kind = g_tks[((g_tkIter + 1) * TokenSize) + Kind];
        if ((kind >= Int && kind <= Void)) {
            ++g_tkIter;
            int data_type = expect_type();
            expect(')');
            cast_expr();
            return data_type;
        }
    }
    return unary_expr();
}
int mul_expr() {
    int data_type = cast_expr();
    while (1) {
        int kind = g_tks[((g_tkIter) * TokenSize) + Kind], opcode;
        if (kind == '*') opcode = Mul;
        else if (kind == '/') opcode = Div;
        else if (kind == '%') opcode = Rem;
        else break;
        ++g_tkIter;
        instruction(Push | (EAX << 24), 0);
        cast_expr();
        instruction(Pop | (EBX << 8), 0);
        instruction(((opcode) | (EAX << 8) | (EBX << 16) | (EAX << 24)), 0);
    }
    return data_type;
}
int add_expr() {
    int data_type = mul_expr();
    while (1) {
        int kind = g_tks[((g_tkIter) * TokenSize) + Kind], opcode;
        if (kind == '+') opcode = Add;
        else if (kind == '-') opcode = Sub;
        else break;
        ++g_tkIter;
        instruction(Push | (EAX << 24), 0);
        int rhs = mul_expr();
        if ((0xFF0000 & data_type) && (0xFF0000 & rhs)) {
            if (data_type != rhs) {
                { printf("error:%d: type mismatch", g_tks[((g_tkIter) * TokenSize) + Ln]); exit(1); };
            }
            if (data_type != (0xFF0000 | Char)) {
                panic("TODO: handle subtraction other than char* - char*");
            }
            data_type = Int;
        }
        instruction(Pop | (EBX << 8), 0);
        if ((0xFF0000 & data_type) && data_type != (0xFF0000 | Char)) { instruction(Mul | (EAX << 8) | (EAX << 16) | (IMME << 24), (4)); }
        instruction(((opcode) | (EAX << 8) | (EBX << 16) | (EAX << 24)), 0);
    }
    return data_type;
}
int shift_expr() {
    int data_type = add_expr();
    while (1) {
        int kind = g_tks[((g_tkIter) * TokenSize) + Kind];
        if (kind != LShift && kind != RShift) break;
        ++g_tkIter;
        instruction(Push | (EAX << 24), 0);
        add_expr();
        instruction(Pop | (EBX << 8), 0);
        instruction(((kind) | (EAX << 8) | (EBX << 16) | (EAX << 24)), 0);
    }
    return data_type;
}
int relation_expr() {
    int data_type = shift_expr();
    while (1) {
        int kind = g_tks[((g_tkIter) * TokenSize) + Kind], opcode;
        if (kind == TkNeq) opcode = Neq;
        else if (kind == TkEq) opcode = Eq;
        else if (kind == '<') opcode = Lt;
        else if (kind == '>') opcode = Gt;
        else if (kind == TkGe) opcode = Ge;
        else if (kind == '<') opcode = Lt;
        else if (kind == TkLe) opcode = Le;
        else break;
        ++g_tkIter;
        instruction(Push | (EAX << 24), 0);
        shift_expr();
        instruction(Pop | (EBX << 8), 0);
        instruction(((opcode) | (EAX << 8) | (EBX << 16) | (EAX << 24)), 0);
    }
    return data_type;
}
int bit_expr() {
    int data_type = relation_expr();
    while (1) {
        int kind = g_tks[((g_tkIter) * TokenSize) + Kind], opcode;
        if (kind == '&') opcode = And;
        else if (kind == '|') opcode = Or;
        else break;
        ++g_tkIter;
        instruction(Push | (EAX << 24), 0);
        relation_expr();
        instruction(Pop | (EBX << 8), 0);
        instruction(((opcode) | (EAX << 8) | (EBX << 16) | (EAX << 24)), 0);
    }
    return data_type;
}
int logical_expr() {
    int data_type = bit_expr();
    while (1) {
        int kind = g_tks[((g_tkIter) * TokenSize) + Kind], opcode;
        if (kind == TkAnd) opcode = Jz;
        else if (kind == TkOr) opcode = Jnz;
        else break;
        ++g_tkIter;
        int skip = g_opCnt;
        instruction(opcode, 0);
        bit_expr();
        g_ops[((skip) * OpSize) + Imme] = g_opCnt;
        continue;
    }
    return data_type;
}
int assign_expr() {
    int data_type = logical_expr();
    while (1) {
        int kind = g_tks[((g_tkIter) * TokenSize) + Kind];
        if (kind == '=') {
            ++g_tkIter;
            instruction(Push | (EDX << 24), 0);
            logical_expr();
            instruction(Pop | (EDX << 8), 0);
            instruction(((Save) | (EDX << 8) | (EAX << 16) | (0 << 24)), data_type == Char ? 1 : 4);
            continue;
        }
        if (kind == TkAddTo) {
            ++g_tkIter;
            instruction(Push | (EDX << 24), 0);
            relation_expr();
            instruction(Pop | (EDX << 8), 0);
            instruction(Load | (EBX << 8) | (EDX << 16), 4);
            if ((0xFF0000 & data_type) && data_type != (0xFF0000 | Char)) { instruction(Mul | (EAX << 8) | (EAX << 16) | (IMME << 24), (4)); }
            instruction(Add | (EAX << 8) | (EBX << 16) | (EAX << 24), (0));
            instruction(((Save) | (EDX << 8) | (EAX << 16) | (0 << 24)), 4);
            continue;
        }
        if (kind == TkSubFrom) {
            ++g_tkIter;
            instruction(Push | (EDX << 24), 0);
            relation_expr();
            instruction(Pop | (EDX << 8), 0);
            instruction(Load | (EBX << 8) | (EDX << 16), 4);
            if ((0xFF0000 & data_type) && data_type != (0xFF0000 | Char)) { instruction(Mul | (EAX << 8) | (EAX << 16) | (IMME << 24), (4)); }
            instruction(Sub | (EAX << 8) | (EBX << 16) | (EAX << 24), (0));
            instruction(((Save) | (EDX << 8) | (EAX << 16) | (0 << 24)), 4);
            continue;
        }
        if (kind == '?') {
            ++g_tkIter;
            int goto_L1 = g_opCnt;
            instruction(Jz, 0);
            int lhs = expr();
            expect(':');
            int goto_L2 = g_opCnt;
            instruction(Jump, g_opCnt + 1);
            g_ops[((goto_L1) * OpSize) + Imme] = g_opCnt;
            int rhs = assign_expr();
            g_ops[((goto_L2) * OpSize) + Imme] = g_opCnt;
            continue;
        }
        break;
    }
    return data_type;
}
int expr() {
    int type = assign_expr();
    while (g_tks[((g_tkIter) * TokenSize) + Kind] == ',') {
        g_tkIter += 1;
        type = assign_expr();
    }
    return type;
}
void stmt() {
    int kind = g_tks[((g_tkIter) * TokenSize) + Kind];
    if (kind == Return) {
        if (g_tks[((++g_tkIter) * TokenSize) + Kind] != ';') { assign_expr(); }
        instruction(Mov | (ESP << 8) | (EBP << 24), 0);
        instruction(Pop | (EBP << 8), 0);
        instruction(Ret, 0);
        expect(';');
        return;
    }
    if (kind == If) {
        ++g_tkIter;
        expect('('); expr(); expect(')');
        int goto_L1 = g_opCnt;
        instruction(Jz, 0);
        stmt();
        if (g_tks[((g_tkIter) * TokenSize) + Kind] != Else) {
            g_ops[((goto_L1) * OpSize) + Imme] = g_opCnt;
            return;
        }
        ++g_tkIter;
        int goto_L2 = g_opCnt;
        instruction(Jump, g_opCnt + 1);
        g_ops[((goto_L1) * OpSize) + Imme] = g_opCnt;
        stmt();
        g_ops[((goto_L2) * OpSize) + Imme] = g_opCnt;
        return;
    }
    if (kind == While) {
        int label_cont = g_opCnt;
        ++g_tkIter;
        expect('('); expr(); expect(')');
        int goto_end = g_opCnt;
        instruction(Jz, 0);
        stmt();
        instruction(Jump, label_cont);
        int label_break = g_opCnt;
        g_ops[((goto_end) * OpSize) + Imme] = label_break;
        int i = g_opCnt - 1;
        while (i > label_cont) {
            if (g_ops[((i) * OpSize) + OpCode] == _BreakStub) { g_ops[((i) * OpSize) + OpCode] = Jump; g_ops[((i) * OpSize) + Imme] = label_break; }
            else if (g_ops[((i) * OpSize) + OpCode] == _ContStub) { g_ops[((i) * OpSize) + OpCode] = Jump; g_ops[((i) * OpSize) + Imme] = label_cont; }
            i -= 1;
        }
        return;
    }
    if (kind == Break) {
        ++g_tkIter;
        instruction(_BreakStub, 0);
        expect(';');
        return;
    }
    if (kind == Cont) {
        ++g_tkIter;
        instruction(_ContStub, 0);
        expect(';');
        return;
    }
    if (kind == '{') {
        enter_scope();
        ++g_tkIter;
        int restore = 0;
        while (g_tks[((g_tkIter) * TokenSize) + Kind] != '}') {
            kind = g_tks[((g_tkIter) * TokenSize) + Kind];
            if ((kind >= Int && kind <= Void)) {
                ++g_tkIter;
                int base_type = kind, varNum = 0;
                while (1) {
                    if (varNum > 0) {
                        expect(',');
                    }
                    int ptr = 0;
                    while (g_tks[((g_tkIter) * TokenSize) + Kind] == '*') { ptr = (ptr << 8) | 0xFF; ++g_tkIter; }
                    int id = expect(Id), prev = g_symCnt - 1;
                    g_syms[((g_symCnt) * SymSize) + Address] = 4;
                    if (prev >= 0 && g_syms[((prev) * SymSize) + Storage] == Local) {
                        g_syms[((g_symCnt) * SymSize) + Address] += g_syms[((prev) * SymSize) + Address];
                    }
                    g_syms[((g_symCnt) * SymSize) + Storage] = Local;
                    g_syms[((g_symCnt) * SymSize) + TkIdx] = id;
                    g_syms[((g_symCnt) * SymSize) + Scope] = g_scopes[g_scopeCnt - 1];
                    g_syms[((g_symCnt) * SymSize) + DType] = (ptr << 16) | base_type;
                    instruction(Sub | (ESP << 8) | (ESP << 16) | (IMME << 24), (4));
                    if (g_tks[((g_tkIter) * TokenSize) + Kind] == '=') {
                        ++g_tkIter;
                        assign_expr();
                        instruction(Sub | (EDX << 8) | (EBP << 16) | (IMME << 24), (g_syms[((g_symCnt) * SymSize) + Address]));
                        instruction(((Save) | (EDX << 8) | (EAX << 16) | (0 << 24)), 4);
                    }
                    ++restore, ++varNum, ++g_symCnt;
                    if (g_tks[((g_tkIter) * TokenSize) + Kind] == ';') { break; }
                }
                ++g_tkIter;
            } else {
                stmt();
            }
        }
        ++g_tkIter;
        if (restore) { instruction(Add | (ESP << 8) | (ESP << 16) | (IMME << 24), (restore << 2)); }
        exit_scope();
        return;
    }
    if (kind == ';') {
        ++g_tkIter;
        return;
    }
    expr();
    expect(';');
    return;
}
void obj() {
    int kind = g_tks[((g_tkIter) * TokenSize) + Kind];
    if (kind == Enum) {
        ++g_tkIter;
        expect('{');
        int val = 0;
        while (g_tks[((g_tkIter) * TokenSize) + Kind] != '}') {
            int idx = expect(Id);
            g_syms[((g_symCnt) * SymSize) + TkIdx] = idx;
            g_syms[((g_symCnt) * SymSize) + Storage] = Const;
            g_syms[((g_symCnt) * SymSize) + DType] = Int;
            g_syms[((g_symCnt) * SymSize) + Scope] = g_scopes[g_scopeCnt - 1];
            if (g_tks[((g_tkIter) * TokenSize) + Kind] == '=') {
                ++g_tkIter;
                idx = expect(CInt);
                val = g_tks[((idx) * TokenSize) + Value];
            }
            g_syms[((g_symCnt++) * SymSize) + Address] = val++;
            if (g_tks[((g_tkIter) * TokenSize) + Kind] == '}') { break; }
            expect(',');
        }
        ++g_tkIter;
        expect(';');
        return;
    }
    int ln = g_tks[((g_tkIter) * TokenSize) + Ln];
    int start = g_tks[((g_tkIter) * TokenSize) + Start];
    int end = g_tks[((g_tkIter++) * TokenSize) + End];
    if (!(kind >= Int && kind <= Void)) {
        { printf("error:%d: unexpected token '%.*s'\n", ln, end - start, start); exit(1); };
    }
    while (g_tks[((g_tkIter) * TokenSize) + Kind] != ';') {
        int data_type = kind, ptr = 0;
        while (g_tks[((g_tkIter) * TokenSize) + Kind] == '*') {
            ptr = (ptr << 8) | 0xFF;
            ++g_tkIter;
        }
        data_type = (ptr << 16) | data_type;
        int id = expect(Id);
        if (g_tks[((g_tkIter) * TokenSize) + Kind] != '(') {
            g_syms[((g_symCnt) * SymSize) + Storage] = Global;
            g_syms[((g_symCnt) * SymSize) + TkIdx] = id;
            g_syms[((g_symCnt) * SymSize) + Scope] = g_scopes[g_scopeCnt - 1];
            g_syms[((g_symCnt) * SymSize) + DType] = data_type;
            *((int*)g_bss) = 0;
            g_syms[((g_symCnt++) * SymSize) + Address] = g_bss;
            g_bss += 4;
            if (g_tks[((g_tkIter) * TokenSize) + Kind] != ';') { expect(','); }
            continue;
        }
        if (streq("main", g_tks[((id) * TokenSize) + Start], 4)) {
            g_entry = g_opCnt;
        } else {
            g_syms[((g_symCnt) * SymSize) + Storage] = Func;
            g_syms[((g_symCnt) * SymSize) + TkIdx] = id;
            g_syms[((g_symCnt) * SymSize) + DType] = data_type;
            g_syms[((g_symCnt) * SymSize) + Scope] = g_scopes[g_scopeCnt - 1];
            g_syms[((g_symCnt++) * SymSize) + Address] = g_opCnt;
        }
        enter_scope();
        expect('(');
        int argCnt = 0, i = 1;
        while (g_tks[((g_tkIter) * TokenSize) + Kind] != ')') {
            if (argCnt > 0) { expect(','); }
            int data_type = expect_type();
            int ptr = 0;
            while (g_tks[((g_tkIter) * TokenSize) + Kind] == '*') { ptr = (ptr << 8) | 0xFF; ++g_tkIter; }
            data_type = (ptr << 16) | data_type;
            g_syms[((g_symCnt) * SymSize) + TkIdx] = expect(Id);
            g_syms[((g_symCnt) * SymSize) + Scope] = g_scopes[g_scopeCnt - 1];
            g_syms[((g_symCnt) * SymSize) + DType] = data_type;
            g_syms[((g_symCnt++) * SymSize) + Storage] = Param;
            ++argCnt;
        }
        expect(')');
        while (i <= argCnt) {
            g_syms[((g_symCnt - i) * SymSize) + Address] = -((i + 1) << 2);
            ++i;
        }
        instruction(Push | (EBP << 24), 0);
        instruction(Mov | (EBP << 8) | (ESP << 24), 0);
        stmt();
        exit_scope();
        return;
    }
    ++g_tkIter;
    return;
}
void gen(int argc, char** argv) {
    enter_scope();
    while (g_tkIter < g_tkCnt) {
        obj();
    }
    int i = 0;
    while (i < g_callCnt) {
        int idx = g_calls[((i) * CallSize) + TkIdx];
        int start = g_tks[((idx) * TokenSize) + Start];
        int end = g_tks[((idx) * TokenSize) + End];
        int ln = g_tks[((idx) * TokenSize) + Ln];
        int len = end - start;
        int found = 0, j = 0;
        while (j < g_symCnt) {
            if (g_syms[((j) * SymSize) + Storage] == Func) {
                int funcIdx = g_syms[((j) * SymSize) + TkIdx];
                if (streq(start, g_tks[((funcIdx) * TokenSize) + Start], len)) {
                    found = 1;
                    g_ops[((g_calls[((i) * CallSize) + InsIdx]) * OpSize) + Imme] = g_syms[((j) * SymSize) + Address];
                    break;
                }
            }
            ++j;
        }
        if (!found) {
            { printf("error:%d: unknown reference to call %.*s\n", ln, len, start); exit(1); };
        }
        ++i;
    }
    exit_scope();
    int argptr = g_bss, it = 0;
    char** argStart = g_bss;
    char* stringStart = argStart + argc;
    while (it < argc) {
        argStart[it] = stringStart;
        char* p = argv[it];
        while (*p) {
            *stringStart++ = *p++;
            ++g_bss;
        }
        *stringStart++ = 0;
        ++g_bss, ++it;
    }
    g_bss = ((g_bss + 3) & -4);
    int entry = g_opCnt;
    instruction(Push | (IMME << 24), argc);
    instruction(Push | (IMME << 24), argptr);
    instruction(Call, g_entry);;
    g_entry = entry;
    return;
}
void dump_code() {
    printf("-------- code --------\n");
    char* regs = "   eaxebxecxedxespebp";
    int pc = 0;
    while (pc < g_opCnt) {
        int op = g_ops[((pc) * OpSize) + OpCode];
        int imme = g_ops[((pc) * OpSize) + Imme];
        int dest = (op & 0xFF00) >> 8;
        int src1 = (op & 0xFF0000) >> 16;
        int src2 = (op & 0xFF000000) >> 24;
        op = op & 0xFF;
        char* width = imme == 4 ? "word" : "byte";
        printf("[ %4d ] ", pc);
        if (op == Mov) {
            if (src2 == IMME) printf("  mov %.*s, %d(0x%08X)\n", 3, regs + 3 * dest, imme, imme);
            else printf("  mov %.*s, %.*s\n", 3, regs + 3 * dest, 3, regs + 3 * src2);
        } else if (op == Ret) {
            printf("  ret\n");
        } else if (op == Add || op == Sub || op == Mul || op == Div || op == Rem) {
            char* opstr = op == Add ? "add" : op == Sub ? "sub" : op == Mul ? "mul" : op == Div ? "div" : "rem";
            if (src2 == IMME) printf("  %s %.*s, %.*s, %d\n", opstr, 3, regs + 3 * dest, 3, regs + 3 * src1, imme);
            else printf("  %s %.*s, %.*s, %.*s\n", opstr, 3, regs + 3 * dest, 3, regs + 3 * src1, 3, regs + 3 * src2);
        } else if (op == Eq || op == Neq || op == Gt || op == Ge || op == Lt || op == Le) {
            char* opstr = op == Eq ? "==" : op == Neq ? "!=" : op == Gt ? ">" : op == Ge ? ">=" : op == Lt ? "<" : "<=";
            printf("  %s %.*s, %.*s, %.*s\n", opstr, 3, regs + 3 * dest, 3, regs + 3 * src1, 3, regs + 3 * src2);
        } else if (op == And) {
            printf("  and %.*s, %.*s, %.*s\n", 3, regs + 3 * dest, 3, regs + 3 * src1, 3, regs + 3 * src2);
        } else if (op == Or) {
            printf("  or %.*s, %.*s, %.*s\n", 3, regs + 3 * dest, 3, regs + 3 * src1, 3, regs + 3 * src2);
        } else if (op == Not) {
            printf("  not %.*s\n", 3, regs + 3 * dest);
        } else if (op == LShift) {
            printf("  lshift %.*s, %.*s, %.*s\n", 3, regs + 3 * dest, 3, regs + 3 * src1, 3, regs + 3 * src2);
        } else if (op == RShift) {
            printf("  rshift %.*s, %.*s, %.*s\n", 3, regs + 3 * dest, 3, regs + 3 * src1, 3, regs + 3 * src2);
        } else if (op == Push) {
            if (src2 == IMME) printf("  push %d(0x%08X)\n", imme, imme);
            else printf("  push %.*s\n", 3, regs + 3 * src2);
        } else if (op == Pop) {
            printf("  pop %.*s\n", 3, regs + 3 * dest);
        } else if (op == Load) {
            printf("  load %.*s, %s[%.*s]\n", 3, regs + 3 * dest, width, 3, regs + 3 * src1);
        } else if (op == Save) {
            printf("  save %s[%.*s], %.*s\n", width, 3, regs + 3 * dest, 3, regs + 3 * src1);
        } else if (op == Jump || op == Jz || op == Jnz || op == Call) {
            char* opstr = op == Jump ? "jmp" : op == Jz ? "jz" : op == Jnz ? "jnz" : "call";
            printf("  %s %d\n", opstr, imme);
        } else if (op == Printf || op == Fopen || op == Fgetc || op == Malloc || op == Exit) {
            char* opstr = op == Printf ? "printf" : op == Fopen ? "fopen" : op == Fgetc ? "fgetc" : op == Malloc ? "malloc" : "exit";
            printf("  %s\n", opstr);
        } else {
            panic("invalid op code");
        }
        ++pc;
    }
    return;
}
int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s file [args...]\n", *argv);
        return 1;
    }
    void* fp = fopen(argv[1], "r");
    if (!fp) {
        printf("file '%s' does not exist\n", argv[1]);
        return 1;
    }
    g_reserved = 2 * (1 << 20) * argc;
    g_ram = malloc(g_reserved);
    int src_reserved = 1 << 18;
    int tk_reserved = 4 * TokenSize * (src_reserved >> 2);
    int sym_reserved = 4 * SymSize * (tk_reserved >> 8);
    int opcode_reserved = 4 * OpSize * (src_reserved >> 3);
    int scope_reserved = 4 * 128;
    int call_reserved = 4 * CallSize * 1024;
    g_src = g_ram + (g_reserved - src_reserved);
    g_tks = g_ram + (g_reserved - src_reserved - tk_reserved);
    g_syms = g_ram + (g_reserved - src_reserved - tk_reserved - sym_reserved);
    g_scopes = g_ram + (g_reserved - src_reserved - tk_reserved - sym_reserved - scope_reserved);
    g_calls = g_ram + (g_reserved - src_reserved - tk_reserved - sym_reserved - scope_reserved - call_reserved);
    g_bss = g_ram + opcode_reserved;
    g_ops = g_ram;
    int src_len = 0, c;
    while ((c = fgetc(fp)) != -1) { g_src[src_len++] = c; }
    g_src[src_len] = 0;
    lex();
    gen(argc - 1, argv + 1);
    ;
    ;
    ;
    ;
    ;
    ;
    g_regs = g_ram + g_reserved - 4 * IMME;
    g_regs[ESP] = g_ram + g_reserved - 4 * IMME;
    int pc = g_entry;
    while (pc < g_opCnt) {
        int op = g_ops[((pc) * OpSize) + OpCode];
        int imme = g_ops[((pc) * OpSize) + Imme];
        int dest = (op & 0xFF00) >> 8;
        int src1 = (op & 0xFF0000) >> 16;
        int src2 = (op & 0xFF000000) >> 24;
        int value = src2 == IMME ? imme : g_regs[src2];
        op = op & 0xFF;
        if (op == Call) {
            g_regs[ESP] -= 4;
            *((int*)g_regs[ESP]) = pc + 1;
            pc = imme;
            continue;
        }
        if ((op == Jump)) {
            pc = imme;
            continue;
        }
        if (op == Jz) {
            if (g_regs[EAX] == 0) pc = imme;
            else pc = pc + 1;
            continue;
        }
        if (op == Jnz) {
            if (g_regs[EAX]) pc = imme;
            else pc = pc + 1;
            continue;
        }
        if (op == Ret) {
            pc = *((int*)g_regs[ESP]);
            g_regs[ESP] += 4;
            continue;
        }
        pc = pc + 1;
        if (op == Mov) { g_regs[dest] = value; }
        else if (op == Push) { g_regs[ESP] -= 4; *((int*)g_regs[ESP]) = value; }
        else if (op == Pop) { g_regs[dest] = *((int*)g_regs[ESP]); g_regs[ESP] += 4; }
        else if (op == Add) { g_regs[dest] = g_regs[src1] + value; }
        else if (op == Sub) { g_regs[dest] = g_regs[src1] - value; }
        else if (op == Mul) { g_regs[dest] = g_regs[src1] * value; }
        else if (op == Div) { g_regs[dest] = g_regs[src1] / value; }
        else if (op == Rem) { g_regs[dest] = g_regs[src1] % value; }
        else if (op == Eq) { g_regs[dest] = g_regs[src1] == value; }
        else if (op == Neq) { g_regs[dest] = g_regs[src1] != value; }
        else if (op == Ge) { g_regs[dest] = g_regs[src1] >= value; }
        else if (op == Gt) { g_regs[dest] = g_regs[src1] > value; }
        else if (op == Le) { g_regs[dest] = g_regs[src1] <= value; }
        else if (op == Lt) { g_regs[dest] = g_regs[src1] < value; }
        else if (op == And) { g_regs[dest] = g_regs[src1] & value; }
        else if (op == Or) { g_regs[dest] = g_regs[src1] | value; }
        else if (op == Not) { g_regs[dest] = !g_regs[dest]; }
        else if (op == LShift) { g_regs[dest] = g_regs[src1] << value; }
        else if (op == RShift) { g_regs[dest] = g_regs[src1] >> value; }
        else if (op == Save) {
            if (imme == 4) *((int*)g_regs[dest]) = g_regs[src1];
            else *((char*)g_regs[dest]) = g_regs[src1];
        }
        else if (op == Load) {
            if (imme == 4) g_regs[dest] = *((int*)g_regs[src1]);
            else g_regs[dest] = *((char*)g_regs[src1]);
        }
        else if (op == Printf) {
            int* p = g_regs[ESP];
            printf((char*)(p[7]), p[6], p[5], p[4], p[3], p[2], p[1], p[0]);
        } else if (op == Fgetc) {
            int* p = g_regs[ESP];
            g_regs[EAX] = fgetc((void*)(p[0]));
        } else if (op == Fopen) {
            int* p = g_regs[ESP];
            g_regs[EAX] = fopen((char*)(p[1]), (char*)(p[0]));
        } else if (op == Malloc) {
            g_regs[EAX] = g_ram + (1 << 20);
        } else if (op == Exit) {
            g_regs[EAX] = *((int*)g_regs[ESP]);
            break;
        } else { panic("Invalid op code"); }
    }
    ;
    ;
    return 0;
}
