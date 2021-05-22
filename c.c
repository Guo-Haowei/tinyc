//- src: cc.c
//-
//- Data Type
//- struct {
//-     int base    : 8; // char, int or void
//-     int ptr     : 8; // char*, int* or void*
//-     int ptr     : 8; // char**, int** or void**
//-     int ptr     : 8;
//- };
//-
//- Virtual Machine
//- Instruction
//- struct {
//-     int op      : 8;
//-     int dest    : 8;
//-     int src1    : 8;
//-     int src2    : 8;
//-     int immediate;
//- };
//-
//- instruction sets (src2 is either register or immediate value)
//-
//- push src2               >-- esp -= 4; [esp] = src2
//- pop dest                >-- dest = [esp]; esp += 4
//- mov dest, src2          >-- dest = src2
//- save dst, src2, byte    >-- [dst] = src2
//- load dst, src1, byte    >-- dst = [src1]
//-
//- add dst, src1, src2     >-- dst = src1 + src2
//- sub dst, src1, src2     >-- dst = src1 - src2
//- mul dst, src1, src2     >-- dst = src1 * src2
//- div dst, src1, src2     >-- dst = src1 / src2
//- rem dst, src1, src2     >-- dst = src1 % src2
//- not dst                 >-- dst = !dst
//-
//- jz src2                 >-- if eax == 0; pc = src2
//- jump src2               >-- pc = src2
//- ret                     >-- pop pc
//- call func               >-- push pc + 1; pc = func
//-
//- printf                  >-- call c printf, push 8 args onto stack

#ifndef NOT_DEVELOPMENT
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#endif // #ifndef NOT_DEVELOPMENT

// definitions
#define MAX_INS (1 << 12)
#define MAX_PRINF_ARGS 8
#define CHUNK_SIZE (1 << 20)

#if defined(TEST) || defined(NOT_DEVELOPMENT)
#define DEVPRINT(...)
#else
#define DEVPRINT(...) fprintf(stderr, __VA_ARGS__)
#endif

#define IS_LETTER(C) ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z'))
#define IS_DIGIT(C) (C >= '0' && C <= '9')
#define IS_HEX(C) (IS_DIGIT(C) || (C >= 'A' && C <= 'F'))
#define IS_WHITESPACE(C) (C == ' ' || C == '\n' || C == '\r' || C == '\t')
#define IS_PUNCT(P, A, B) (*P == A && P[1] == B)
#define IS_TYPE(KIND) (KIND >= Int && KIND <= Void)
#define ALIGN(x) ((x + 3) & -4)
#define COMPILE_ERROR(...) { printf(__VA_ARGS__); exit(1); }
#define PUSH(REG, VAL) instruction(Push | (REG << 24), VAL)
#define POP(REG) instruction(Pop | (REG << 8), 0)
#define MOV(DEST, SRC, IMME) instruction(Mov | (DEST << 8) | (SRC << 24), IMME)
#define ADD(DEST, SRC1, SRC2, VAL) instruction(Add | (DEST << 8) | (SRC1 << 16) | (SRC2 << 24), (VAL))
#define SUB(DEST, SRC1, SRC2, VAL) instruction(Sub | (DEST << 8) | (SRC1 << 16) | (SRC2 << 24), (VAL))
#define MUL(DEST, SRC1, SRC2, VAL) instruction(Mul | (DEST << 8) | (SRC1 << 16) | (SRC2 << 24), (VAL))
#define CALL(ENTRY) instruction(Call, ENTRY);
#define LOADB(DEST, SRC) instruction(Load | (DEST << 8) | (SRC << 16), 1)
#define LOADW(DEST, SRC) instruction(Load | (DEST << 8) | (SRC << 16), 4)
#define CHAR_PTR (0xFF0000 | Char)
#define VOID_PTR (0xFF0000 | Void)
#define IS_PTR(TYPE) (0xFF0000 & TYPE)

enum { _TK_OFFSET = 128,
       CInt, Id, CStr, CChar,
       TkNeq, TkEq, TkGe, TkLe,
       TkAddTo, TkSubFrom, TkInc, TkDec, TkAnd, TkOr, LShift, RShift,
       Int, Char, Void,
       Break, Cont, Do, Else, Enum, For, If, Return, Sizeof, While,
       Printf, Fopen, Fgetc, Malloc, Memset, Exit,
       Add, Sub, Mul, Div, Rem,
       Mov, Push, Pop, Load, Save,
       Neq, Eq, Gt, Ge, Lt, Le, And, Or,
       Not, Ret, Jz, Jnz, Jump, Call,
       
       _BreakStub, _ContStub, };
enum { Undefined, Global, Param, Local, Func, Const, };
enum { EAX = 1, EBX, ECX, EDX, ESP, EBP, IMME, };

struct Token {
    int kind;
    int value;
    int ln;
    char* start;
    char* end;
};

struct Symbol {
    int tkIdx;
    int scope;
    int data_type;
    int storage;
    int address;
};

struct Ins {
    int op;
    int imme;
};

char *g_src;

// tokens
struct Token* g_tks;
int g_tkCnt, g_tkIter;

int g_regs[IMME];
struct Ins g_instructs[MAX_INS];
int g_insCnt;
int g_entry;
int g_pc; // program counter

char* ram;
int memory;
int g_bss;

#define MAX_SCOPE 128
int g_scopeId;
int g_scopes[MAX_SCOPE];
int scopeCnt;

struct Symbol* syms;
int g_symCnt;

#if !defined(TEST) && !defined(NOT_DEVELOPMENT)
int expr();
#endif

// utility
void panic(char* fmt) {
    printf("[panic] %s\n", fmt);
    exit(1);
}

void lex() {
    int ln = 1;
    char *p = g_src, *kw = "int char void break continue do else enum for if "
                           "return sizeof while printf fopen fgetc malloc memset exit ";
    while (*p) {
        if (*p == '#' || (*p == '/' && *(p + 1) == '/')) {
            while (*p && *p != 10) ++p;
        } else if (IS_WHITESPACE(*p)) {
            ln += (*p == 10);
            p = p + 1;
        } else {
            g_tks[g_tkCnt].ln = ln;
            g_tks[g_tkCnt].start = p;

            // id or keyword
            if (IS_LETTER(*p) || *p == '_') {
                g_tks[g_tkCnt].kind = Id;
                for (++p; IS_LETTER(*p) || IS_DIGIT(*p) || *p == '_'; ++p);
                char *p0 = kw, *p1 = kw;
                for (int kind = Int; (p1 = strchr(p0, ' ')); p0 = p1 + 1, ++kind) {
                    if (strncmp(p0, g_tks[g_tkCnt].start, p1 - p0) == 0) {
                        g_tks[g_tkCnt].kind = kind;
                        break;
                    }
                }
                g_tks[g_tkCnt++].end = p;
            } else if (*p == '0' && p[1] == 'x') {
                g_tks[g_tkCnt].kind = CInt;
                int result = 0;
                for (p += 2; IS_HEX(*p); ++p) {
                    result = (result << 4) + ((*p < 'A') ? (*p - '0') : (*p - 55));
                }
                g_tks[g_tkCnt].value = result;
                g_tks[g_tkCnt++].end = p;
            } else if (IS_DIGIT(*p)) {
                g_tks[g_tkCnt].kind = CInt;
                int result = 0;
                for (; IS_DIGIT(*p); ++p) { result = result * 10 + (*p - '0'); }
                g_tks[g_tkCnt].value = result;
                g_tks[g_tkCnt++].end = p;
            } else if (*p == '"') {
                g_tks[g_tkCnt].kind = CStr;
                for (++p; *p != '"'; ++p);
                g_tks[g_tkCnt++].end = ++p;
            } else if (*p == 39) { // ascii '''
                g_tks[g_tkCnt].kind = CChar;
                g_tks[g_tkCnt].value = p[1];
                g_tks[g_tkCnt++].end = (p += 3);
            } else {
                g_tks[g_tkCnt].kind = *p;

                if (IS_PUNCT(p, '=', '=')) { g_tks[g_tkCnt].kind = TkEq; ++p; }
                else if (IS_PUNCT(p, '!', '=')) { g_tks[g_tkCnt].kind = TkNeq; ++p; }
                else if (IS_PUNCT(p, '&', '&')) { g_tks[g_tkCnt].kind = TkAnd; ++p; }
                else if (IS_PUNCT(p, '|', '|')) { g_tks[g_tkCnt].kind = TkOr; ++p; }
                else if (*p == '+') {
                    if (p[1] == '+') { g_tks[g_tkCnt].kind = TkInc; ++p; }
                    else if (p[1] == '=') { g_tks[g_tkCnt].kind = TkAddTo; ++p; }
                } else if (*p == '-') {
                    if (p[1] == '-') { g_tks[g_tkCnt].kind = TkDec; ++p; }
                    else if (p[1] == '=') { g_tks[g_tkCnt].kind = TkSubFrom; ++p; }
                } else if (*p == '>') {
                    if (p[1] == '=') { g_tks[g_tkCnt].kind = TkGe; ++p; }
                    else if (p[1] == '>') { g_tks[g_tkCnt].kind = RShift; ++p; }
                } else if (*p == '<') {
                    if (p[1] == '=') { g_tks[g_tkCnt].kind = TkLe; ++p; }
                    else if (p[1] == '<') { g_tks[g_tkCnt].kind = LShift; ++p; }
                }

                g_tks[g_tkCnt++].end = ++p;
            }
        }
    }
}

void dump_tokens() {
    int indent = 0;
    for (int i = 0, ln = 0; i < g_tkCnt; ++i) {
        int tkln = g_tks[i].ln;
        int kind = g_tks[i].kind;
        char* start = g_tks[i].start;
        char* end = g_tks[i].end;
        int len = end - start;
        if (kind == '{') { indent += 1; }
        else if (kind == '}') { indent -= 1; }
        if (ln != tkln) {
            printf("\n%-3d:%.*s", tkln, indent * 4, "                                        ");
            ln = tkln;
        }
        char* names = "Int   Char  Void  Break Cont  Do    Else  Enum  For   "
                      "If    Ret   SizeofWhile Print Open  MallocMemsetExit  ";
        printf("%.*s", len, start);
        if (kind >= Int) {
            char *p = names + 6 * (kind - Int);
            printf("{");
            for (int i = 0; i < 6; ++i, ++p) {
                if (*p == ' ') break;
                printf("%c", *p);
            }
            printf("}");
        }
        printf(" ");
    }
    printf("\n");
}


void enter_scope() {
    if (scopeCnt >= MAX_SCOPE) {
        panic("scope overflow");
    }

    g_scopes[scopeCnt++] = ++g_scopeId;
}

void exit_scope() {
    if (scopeCnt <= 0) {
        panic("scope overflow");
    }

    for (int i = g_symCnt - 1; syms[i].scope == g_scopes[scopeCnt - 1]; --i) {
        --g_symCnt;
    }

    --scopeCnt;
}

int expect(int kind) {
    if (g_tks[g_tkIter].kind != kind) {
        COMPILE_ERROR("error:%d: expected token '%c'(%d), got '%.*s'\n",
            g_tks[g_tkIter].ln,
            kind < 128 ? kind : ' ',
            kind,
            g_tks[g_tkIter].end - g_tks[g_tkIter].start,
            g_tks[g_tkIter].start);
    }
    return g_tkIter++;
}

int expect_type() {
    int base_type = g_tks[g_tkIter].kind;
    if (IS_TYPE(base_type)) {
        ++g_tkIter;
        int ptr = 0;
        for (; g_tks[g_tkIter].kind == '*'; ++g_tkIter)
            ptr = (ptr << 8) | 0xFF;
        return (ptr << 16) | base_type;
    } else {
        COMPILE_ERROR("error:%d: expected type specifier, got '%.*s'\n",
            g_tks[g_tkIter].ln,
            g_tks[g_tkIter].end - g_tks[g_tkIter].start,
            g_tks[g_tkIter].start);
    }
}

void instruction(int op, int imme) {
    if (g_insCnt >= MAX_INS) {
        panic("instruction overflow");
    }

    g_instructs[g_insCnt].op = op;
    g_instructs[g_insCnt].imme = imme;
    ++g_insCnt;
}

#define OP2(op, dest) ((op) | (dest << 8))
#define OP(op, dest, src1, src2) ((op) | (dest << 8) | (src1 << 16) | (src2 << 24))

struct CallToResolve {
    int insIdx;
    int tkIdx;
};

struct CallToResolve g_calls[256];
int g_callNum;

int primary_expr() {
    int tkIdx = g_tkIter++;
    char* start = g_tks[tkIdx].start;
    char* end = g_tks[tkIdx].end;
    int ln = g_tks[tkIdx].ln;
    int kind = g_tks[tkIdx].kind;
    int len = end - start;
    int value = g_tks[tkIdx].value;
    if (kind == CInt || kind == CChar) {
        MOV(EAX, IMME, value);
        return Int;
    }
    
    if (kind == CStr) {
        MOV(EAX, IMME, g_bss);
        for (int i = 1; i < len - 1; ++i) {
            int c = start[i];
            if (c == 92) { // '\'
                c = start[++i];
                if (c == 'n') c = 10;
                else panic("handle escape sequence");
            }
            *((char*)g_bss++) = c;
        }
        *((char*)g_bss++) = 0;
        g_bss = ALIGN(g_bss);
        return CHAR_PTR;
    }

    if (kind == '(') {
        int data_type = expr();
        expect(')');
        return data_type;
    }
    
    if (kind == Id) {
        if (g_tks[g_tkIter].kind == '(') {
            ++g_tkIter;
            int argc;
            for (argc = 0; g_tks[g_tkIter].kind != ')'; ++argc) {
                if (argc > 0) expect(',');
                assign_expr();
                PUSH(EAX, 0);
            }

            g_calls[g_callNum].insIdx = g_insCnt;
            g_calls[g_callNum++].tkIdx = tkIdx;

            CALL(0);
            if (argc) { ADD(ESP, ESP, IMME, argc << 2); }
            expect(')');
            return Int;
        }

        int address = 0, type = Undefined, data_type = 0;
        for (int i = g_symCnt - 1; i >= 0; --i) {
            int tmp = syms[i].tkIdx;
            int tmplen = g_tks[tmp].end - g_tks[tmp].start;
            if (len == tmplen && strncmp(g_tks[tmp].start, start, len) == 0) {
                address = syms[i].address;
                type = syms[i].storage;
                data_type = syms[i].data_type;
                break;
            }
        }

        if (type == Global) {
            panic("TODO: implement globlal variable");
        }

        if (type == Const) {
            MOV(EAX, IMME, address);
            return Int;
        }

        if (type == Undefined) {
            COMPILE_ERROR("error:%d: '%.*s' undeclared\n", ln, len, start);
        }

        SUB(EDX, EBP, IMME, address);
        LOADW(EAX, EDX);
        return data_type;
    }

    if (kind == Printf) {
        expect('(');
        int argc = 0;
        for (; g_tks[g_tkIter].kind != ')'; ++argc) {
            if (argc > 0) expect(',');
            assign_expr();
            PUSH(EAX, 0);
        }
        if (argc > MAX_PRINF_ARGS) panic("printf supports at most %d args");
        SUB(ESP, ESP, IMME, (MAX_PRINF_ARGS - argc) << 2);
        instruction(Printf, argc);
        ADD(ESP, ESP, IMME, MAX_PRINF_ARGS << 2);
        expect(')');
        return Int;
    }

    if (kind == Fopen) {
        expect('(');
        assign_expr();
        PUSH(EAX, 0);
        expect(',');
        assign_expr();
        PUSH(EAX, 0);
        instruction(Fopen, 0);
        ADD(ESP, ESP, IMME, 8);
        expect(')');
        return VOID_PTR;
    }

    if (kind == Fgetc) {
        expect('(');
        assign_expr();
        PUSH(EAX, 0);
        instruction(Fgetc, 0);
        ADD(ESP, ESP, IMME, 4);
        expect(')');
        return Int;
    }

    if (kind == Malloc) {
        expect('(');
        assign_expr();
        PUSH(EAX, 0);
        instruction(Malloc, 0);
        ADD(ESP, ESP, IMME, 4);
        expect(')');
        return VOID_PTR;
    }

    COMPILE_ERROR("error:%d: expected expression, got '%.*s'\n", ln, len, start);
    return Int;
}

int post_expr() {
    int data_type = primary_expr();
    for (;;) {
        int kind = g_tks[g_tkIter].kind;
        int ln = g_tks[g_tkIter].ln;
        if (kind == '[') {
            ++g_tkIter;
            if (!IS_PTR(data_type)) {
                COMPILE_ERROR("error:%d: attempted to dereference a non-pointer type 0x%X\n", ln, data_type);
            }
            PUSH(EAX, 0);
            assign_expr();
            int is_charptr = data_type == CHAR_PTR;
            if (!is_charptr) {
                MUL(EAX, EAX, IMME, 4);
            }
            POP(EBX);
            ADD(EAX, EBX, EAX, 0);
            if (is_charptr) LOADB(EAX, EAX);
            else LOADW(EAX, EAX);
            expect(']');
            data_type = ((data_type >> 8) & 0xFF0000) | ((data_type & 0xFFFF));
        } else {
            break;
        }
    }
    return data_type;
}

int unary_expr() {
    int kind = g_tks[g_tkIter].kind;
    int ln = g_tks[g_tkIter].ln;
    if (kind == '!') {
        ++g_tkIter;
        int data_type = unary_expr();
        instruction(OP(Not, EAX, 0, 0), 0);
        return data_type;
    }
    if (kind == '+') {
        ++g_tkIter;
        return unary_expr();
    }
    if (kind == '-') {
        ++g_tkIter;
        int data_type = unary_expr();
        MOV(EBX, IMME, 0);
        SUB(EAX, EBX, EAX, 0);
        return data_type;
    }
    if (kind == '*') {
        ++g_tkIter;
        int data_type = unary_expr();
        if (!IS_PTR(data_type)) {
            COMPILE_ERROR("error:%d: attempted to dereference a non-pointer type 0x%X\n", ln, data_type);
        }

        MOV(EDX, EAX, 0);
        if (data_type == CHAR_PTR) LOADB(EAX, EDX);
        else LOADW(EAX, EDX);
        return ((data_type >> 8) & 0xFF0000) | (0xFFFF & data_type);
    }
    if (kind == '&') {
        panic("implement &a");
    }
    return post_expr();
}

int mul_expr() {
    int data_type = unary_expr();
    while (1) {
        int optk = g_tks[g_tkIter].kind; int opcode;
        if (optk == '*') opcode = Mul;
        else if (optk == '/') opcode = Div;
        else if (optk == '%') opcode = Rem;
        else break;
        ++g_tkIter;
        PUSH(EAX, 0);
        unary_expr();
        POP(EBX);
        instruction(OP(opcode, EAX, EBX, EAX), 0);
    }

    return data_type;
}

int add_expr() {
    int data_type = mul_expr();
    while (1) {
        int optk = g_tks[g_tkIter].kind; int opcode;
        if (optk == '+') opcode = Add;
        else if (optk == '-') opcode = Sub;
        else break;
        ++g_tkIter;
        PUSH(EAX, 0);
        mul_expr();
        POP(EBX);
        if (IS_PTR(data_type) && data_type != CHAR_PTR) { MUL(EAX, EAX, IMME, 4); }
        instruction(OP(opcode, EAX, EBX, EAX), 0);
    }

    return data_type;
}

int shift_expr() {
    int data_type = add_expr();
    while (1) {
        int kind = g_tks[g_tkIter].kind;
        if (kind != LShift && kind != RShift) break;
        ++g_tkIter;
        PUSH(EAX, 0);
        add_expr();
        POP(EBX);
        instruction(OP(kind, EAX, EBX, EAX), 0);
    }
    return data_type;
}


int relation_expr() {
    int data_type = shift_expr();
    while (1) {
        int kind = g_tks[g_tkIter].kind; int opcode;
        if (kind == TkNeq) opcode = Neq;
        else if (kind == TkEq) opcode = Eq;
        else if (kind == '<') opcode = Lt;
        else if (kind == '>') opcode = Gt;
        else if (kind == TkGe) opcode = Ge;
        else if (kind == '<') opcode = Lt;
        else if (kind == TkLe) opcode = Le;
        else break;
        ++g_tkIter;
        PUSH(EAX, 0);
        shift_expr();
        POP(EBX);
        instruction(OP(opcode, EAX, EBX, EAX), 0);
    }
    return data_type;
}

int bit_expr() {
    int data_type = relation_expr();
    while (1) {
        int kind = g_tks[g_tkIter].kind, opcode;
        if (kind == '&') opcode = And;
        else if (kind == '|') opcode = Or;
        else break;
        ++g_tkIter;
        PUSH(EAX, 0);
        relation_expr();
        POP(EBX);
        instruction(OP(opcode, EAX, EBX, EAX), 0);
    }
    return data_type;
}

int logical_expr() {
    int data_type = bit_expr();
    while (1) {
        int kind = g_tks[g_tkIter].kind, opcode;
        if (kind == TkAnd) opcode = Jz;
        else if (kind == TkOr) opcode = Jnz;
        else break;

        ++g_tkIter;
        int skip = g_insCnt;
        instruction(opcode, 0);
        bit_expr();
        g_instructs[skip].imme = g_insCnt;
        continue;
    }

    return data_type;
}

int assign_expr() {
    int data_type = logical_expr();
    while (1) {
        int kind = g_tks[g_tkIter].kind;
        if (kind == '=') {
            ++g_tkIter;
            PUSH(EDX, 0);
            logical_expr();
            POP(EDX);
            instruction(OP(Save, EDX, EAX, 0), data_type == Char ? 1 : 4);
            continue;
        }

        if (kind == TkAddTo) {
            ++g_tkIter;
            PUSH(EDX, 0);
            int rhs = relation_expr();
            POP(EDX);
            LOADW(EBX, EDX);
            if (IS_PTR(rhs) && rhs != CHAR_PTR) { MUL(EAX, EAX, IMME, 4); }
            ADD(EAX, EBX, EAX, 0);
            instruction(OP(Save, EDX, EAX, 0), 4);
            continue;
        }

        if (kind == TkSubFrom) {
            ++g_tkIter;
            PUSH(EDX, 0);
            int rhs = relation_expr();
            POP(EDX);
            LOADW(EBX, EDX);
            if (IS_PTR(rhs) && rhs != CHAR_PTR) { MUL(EAX, EAX, IMME, 4); }
            SUB(EAX, EBX, EAX, 0);
            instruction(OP(Save, EDX, EAX, 0), 4);
            continue;
        }

        if (kind == '?') {
            ++g_tkIter;
            int goto_L1 = g_insCnt;
            instruction(Jz, 0);
            int lhs = expr();
            expect(':');
            int goto_L2 = g_insCnt;
            instruction(Jump, g_insCnt + 1);
            g_instructs[goto_L1].imme = g_insCnt;
            int rhs = assign_expr();
            g_instructs[goto_L2].imme = g_insCnt;
            continue;
        }

        break;
    }

    return data_type;
}

int expr() {
    int type = assign_expr();
    while (g_tks[g_tkIter].kind == ',') {
        g_tkIter += 1;
        type = assign_expr();
    }
    return type;
}

void stmt() {
    int kind = g_tks[g_tkIter].kind;
    if (kind == Return) {
        if (g_tks[++g_tkIter].kind != ';') assign_expr();
        MOV(ESP, EBP, 0);
        POP(EBP);
        instruction(Ret, 0);
        expect(';');
        return;
    }

    if (kind == If) {
        //     eax == 0; goto L1 |     eax == 0; goto L1
        //     ...               |     ...
        //     goto L2           | L1: ...
        // L1: ...               |
        // L2: ...               |
        ++g_tkIter;
        expect('('); expr(); expect(')');
        int goto_L1 = g_insCnt;
        instruction(Jz, 0);
        stmt();

        if (g_tks[g_tkIter].kind != Else) {
            g_instructs[goto_L1].imme = g_insCnt;
            return;
        }

        ++g_tkIter; // skip else
        int goto_L2 = g_insCnt;
        instruction(Jump, g_insCnt + 1);
        g_instructs[goto_L1].imme = g_insCnt;
        stmt();
        g_instructs[goto_L2].imme = g_insCnt;
        return;
    }

    if (kind == While) {
        // CONT: ...
        //       eax == 0; goto BREAK
        //       ...
        //       jump CONT;
        // BREAK:
        int label_cont = g_insCnt;
        ++g_tkIter;
        expect('('); expr(); expect(')');
        int goto_end = g_insCnt;
        instruction(Jz, 0);
        stmt();
        instruction(Jump, label_cont);
        int label_break = g_insCnt;
        g_instructs[goto_end].imme = label_break;
        int i = g_insCnt - 1;
        while (i > label_cont) {
            if (g_instructs[i].op == _BreakStub) { g_instructs[i].op = Jump; g_instructs[i].imme = label_break; }
            else if (g_instructs[i].op == _ContStub) { g_instructs[i].op = Jump; g_instructs[i].imme = label_cont; }
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
        while (g_tks[g_tkIter].kind != '}') {
            kind = g_tks[g_tkIter].kind;
            if (IS_TYPE(kind)) {
                ++g_tkIter;
                int base_type = kind, varNum = 0;
                do {
                    if (varNum > 0) {
                        expect(',');
                    }

                    int ptr = 0;
                    for (; g_tks[g_tkIter].kind == '*'; ++g_tkIter)
                        ptr = (ptr << 8) | 0xFF;

                    int id = expect(Id);
                    // char* start = g_tks[id].start;
                    int prev = g_symCnt - 1;
                    syms[g_symCnt].address = 4;

                    if (prev >= 0 && syms[prev].storage == Local) {
                        syms[g_symCnt].address += syms[prev].address;
                    }

                    syms[g_symCnt].storage = Local;
                    syms[g_symCnt].tkIdx = id;
                    syms[g_symCnt].scope = g_scopes[scopeCnt - 1];
                    syms[g_symCnt].data_type = (ptr << 16) | base_type;
                    ++g_symCnt;

                    SUB(ESP, ESP, IMME, 4);
                    if (g_tks[g_tkIter].kind == '=') {
                        ++g_tkIter;
                        assign_expr();
                        instruction(OP(Save, ESP, EAX, 0), 4);
                    }

                    ++restore, ++varNum;
                } while (g_tks[g_tkIter].kind != ';');

                ++g_tkIter;
            } else {
                stmt();
            }
        }
        ++g_tkIter;

        if (restore) { ADD(ESP, ESP, IMME, restore << 2); }
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

// an object could be a global variable, an enum or a function
void obj() {
    if (g_tks[g_tkIter].kind == Enum) {
        ++g_tkIter;
        expect('{');
        int val = 0;
        while (g_tks[g_tkIter].kind != '}') {
            int idx = expect(Id);
            syms[g_symCnt].tkIdx = idx;
            syms[g_symCnt].storage = Const;
            syms[g_symCnt].data_type = Int;
            syms[g_symCnt].scope = g_scopes[scopeCnt - 1];

            if (g_tks[g_tkIter].kind == '=') {
                ++g_tkIter;
                idx = expect(CInt);
                val = g_tks[idx].value;
            }

            syms[g_symCnt].address = val;
            ++g_symCnt;
            ++val;

            expect(','); // force comma
        }
        ++g_tkIter;
        expect(';');
        return;
    }

    int data_type = expect_type();
    int id = expect(Id);

    if (g_tks[g_tkIter].kind == '(') {
        if (strncmp("main", g_tks[id].start, 4) == 0) {
            g_entry = g_insCnt;
        } else {
            syms[g_symCnt].storage = Func;
            syms[g_symCnt].tkIdx = id;
            syms[g_symCnt].data_type = data_type;
            syms[g_symCnt].scope = g_scopes[scopeCnt - 1];
            syms[g_symCnt].address = g_insCnt;
            ++g_symCnt;
        }

        enter_scope();
        expect('(');
        int argCnt = 0;
        while (g_tks[g_tkIter].kind != ')') {
            if (argCnt > 0) {
                expect(',');
            }

            int data_type = expect_type();
            int ptr = 0;
            for (; g_tks[g_tkIter].kind == '*'; ++g_tkIter) { ptr = (ptr << 8) | 0xFF; }
            data_type = (ptr << 16) | data_type;

            syms[g_symCnt].tkIdx = expect(Id);
            syms[g_symCnt].scope = g_scopes[scopeCnt - 1];
            syms[g_symCnt].data_type = data_type;
            syms[g_symCnt++].storage = Param;
            ++argCnt;
        }
        expect(')');
        for (int i = 1; i <= argCnt; i = i + 1) {
            syms[g_symCnt - i].address = -((i + 1) << 2);
        }

        // save frame
        PUSH(EBP, 0);
        MOV(EBP, ESP, 0);
        stmt();
        exit_scope();
        return;
    }

    panic("TODO: implement global variable");
}

void gen() {
    enter_scope();

    while (g_tkIter < g_tkCnt) {
        obj();
    }

    // resolve calls
    if (g_callNum > (sizeof(g_calls) / sizeof(g_calls[0]))) {
        panic("Call overflow");
    }

    for (int i = 0; i < g_callNum; ++i) {
        int idx = g_calls[i].tkIdx;
        char* start = g_tks[idx].start;
        char* end = g_tks[idx].end;
        int ln = g_tks[idx].ln;
        int len = end - start;

        int found = 0;
        for (int j = 0; j < g_symCnt; ++j) {
            if (syms[j].storage == Func) {
                int funcIdx = syms[j].tkIdx;
                if (strncmp(start, g_tks[funcIdx].start, len) == 0) {
                    found = 1;
                    g_instructs[g_calls[i].insIdx].imme = syms[j].address;
                    break;
                }
            }
        }

        if (!found) {
            COMPILE_ERROR("error:%d: unknown reference to call %.*s\n", ln, len, start);
        }

    }

    exit_scope();
}

void exec() {
    int pc = g_entry;
    while (pc < g_insCnt) {
        int op = g_instructs[pc].op;
        int imme = g_instructs[pc].imme;
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
            g_regs[EAX] = ram + CHUNK_SIZE;
        } else { panic("Invalid op code"); }
    }
}

void dump_code() {
    char* regs = "   eaxebxecxedxespebp";
    #define REG2STR(REG) 3, regs + 3 * REG
    for (int pc = 0; pc < g_insCnt; ++pc) {
        int op = g_instructs[pc].op;
        int imme = g_instructs[pc].imme;
        int dest = (op & 0xFF00) >> 8;
        int src1 = (op & 0xFF0000) >> 16;
        int src2 = (op & 0xFF000000) >> 24;
        op = op & 0xFF;
        char* width = imme == 4 ? "word" : "byte";
        printf("[ %4d ] ", pc);
        if (op == Mov) {
            if (src2 == IMME) printf("  mov %.*s, %d(0x%08X)\n", REG2STR(dest), imme, imme);
            else printf("  mov %.*s, %.*s\n", REG2STR(dest), REG2STR(src2));
        } else if (op == Ret) {
            printf("  ret\n");
        } else if (op == Add || op == Sub || op == Mul || op == Div || op == Rem) {
            char* opstr = op == Add ? "add" : op == Sub ? "sub" : op == Mul ? "mul" : op == Div ? "div" : "rem";
            if (src2 == IMME) printf("  %s %.*s, %.*s, %d\n", opstr, REG2STR(dest), REG2STR(src1), imme);
            else printf("  %s %.*s, %.*s, %.*s\n", opstr, REG2STR(dest), REG2STR(src1), REG2STR(src2));
        } else if (op == Eq || op == Neq || op == Gt || op == Ge || op == Lt || op == Le) {
            char* opstr = op == Eq ? "==" : op == Neq ? "!=" : op == Gt ? ">" : op == Ge ? ">=" : op == Lt ? "<" : "<=";
            printf("  %s %.*s, %.*s, %.*s\n", opstr, REG2STR(dest), REG2STR(src1), REG2STR(src2));
        } else if (op == And) {
            printf("  and %.*s, %.*s, %.*s\n", REG2STR(dest), REG2STR(src1), REG2STR(src2));
        } else if (op == Or) {
            printf("  or %.*s, %.*s, %.*s\n", REG2STR(dest), REG2STR(src1), REG2STR(src2));
        } else if (op == Not) {
            printf("  not %.*s\n", REG2STR(dest));
        } else if (op == LShift) {
            printf("  lshift %.*s, %.*s, %.*s\n", REG2STR(dest), REG2STR(src1), REG2STR(src2));
        } else if (op == RShift) {
            printf("  rshift %.*s, %.*s, %.*s\n", REG2STR(dest), REG2STR(src1), REG2STR(src2));
        } else if (op == Push) {
            if (src2 == IMME) printf("  push %d(0x%08X)\n", imme, imme);
            else printf("  push %.*s\n", REG2STR(src2));
        } else if (op == Pop) {
            printf("  pop %.*s\n", REG2STR(dest));
        } else if (op == Load) {
            printf("  load %.*s, %s[%.*s]\n", REG2STR(dest), width, REG2STR(src1));
        } else if (op == Save) {
            printf("  save %s[%.*s], %.*s\n", width, REG2STR(dest), REG2STR(src1));
        } else if (op == Jump || op == Jz || op == Jnz || op == Call) {
            char* opstr = op == Jump ? "jmp" : op == Jz ? "jz" : op == Jnz ? "jnz" : "call";
            printf("  %s %d\n", opstr, imme);
        } else if (op == Printf || op == Fopen || op == Fgetc || op == Malloc) {
            char* opstr = op == Printf ? "printf" : op == Fopen ? "fopen" : op == Fgetc ? "fgetc" : "malloc";
            printf("  %s\n", opstr);
        } else {
            panic("invalid op code");
        }
    }
}

void entry(int argc, char** argv) {
    int argptr = g_bss;
    char** argStart = g_bss;
    char* stringStart = argStart + argc;
    for (int i = 0; i < argc; ++i) {
        argStart[i] = stringStart;
        for (char* p = argv[i]; *p; ++p) {
            *stringStart++ = *p;
            ++g_bss;
        }
        *stringStart++ = 0;
        ++g_bss;
    }

    g_bss = ALIGN(g_bss);

    // start
    int entry = g_insCnt;
    PUSH(IMME, argc);
    PUSH(IMME, argptr);
    CALL(g_entry);

    g_entry = entry;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s file [args...]\n", *argv);
        return 1;
    }

    // initialization
    memory = 2 * CHUNK_SIZE * argc;
    ram = malloc(memory);

    // store source to buffer
    void* fp = fopen(argv[1], "r");
    if (!fp) {
        printf("file '%s' does not exist\n", argv[1]);
        return 1;
    }

    int len = 1 << 18;
    g_src = ram + (memory - len); // pad

    int i = 0;
    for (int c; (c = fgetc(fp)) != -1; ++i) { g_src[i] = c; }
    g_src[i] = 0;
    DEVPRINT("source len: %d\n", i);

    g_tks = calloc(len, sizeof(struct Token));
    syms = calloc(len, sizeof(struct Symbol));

    i = len * (1 + sizeof(struct Token) + sizeof(struct Symbol));

    lex();
#if !defined(NOT_DEVELOPMENT) && !defined(TEST)
    DEVPRINT("-------- lex --------\n");
    dump_tokens();
    DEVPRINT("token count: %d\n", g_tkCnt);
#endif

    g_bss = ram;
    g_regs[ESP] = ram + memory;

    gen();
    entry(argc - 1, argv + 1);
#if !defined(NOT_DEVELOPMENT) && !defined(TEST)
    DEVPRINT("-------- code --------\n");
    dump_code();
#endif

    DEVPRINT("-------- exec --------\n");
    exec();

    free(ram);
    free(g_tks);

    DEVPRINT("-------- exiting --------\n");
    DEVPRINT("script '%s' exit with code %d\n", argv[1], g_regs[EAX]);
    return 0;
}
