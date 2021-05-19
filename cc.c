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
//-
//- add dst, src1, src2     >-- dst = src1 + src2
//- sub dst, src1, src2     >-- dst = src1 - src2
//- mul dst, src1, src2     >-- dst = src1 * src2
//- div dst, src1, src2     >-- dst = src1 / src2
//- rem dst, src1, src2     >-- dst = src1 % src2
//-
//- save dst, src2, byte    >-- [dst] = src2
//- load dst, src1, byte    >-- dst = [src1]
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
#define RAM_SIZE (1 << 20)
#define MAX_INS (1 << 12)
#define MAX_PRINF_ARGS 8

//#define switch @
//#define case @
//#define default @

#if defined(TEST) || defined(NOT_DEVELOPMENT)
#define DEVPRINT(...)
#else
#define DEVPRINT(...) fprintf(stderr, __VA_ARGS__)
#endif
#define ERROR(...) { printf("\n%s:", g_prog); printf(__VA_ARGS__); exit(1); }

/// TODO: && elimintaion
#define IS_LETTER(C) ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z'))
#define IS_DIGIT(C) (C >= '0' && C <= '9')
#define IS_HEX(C) (IS_DIGIT(C) || (C >= 'A' && C <= 'F'))
#define IS_WHITESPACE(C) (C == ' ' || C == '\n' || C == '\r' || C == '\t')
#define IS_PUNCT(P, A, B) (*P == A && P[1] == B)
#define IS_TYPE(KIND) (KIND >= INT && KIND <= VOID)
#define ALIGN(x) ((x + 3) & -4)

// Token Kind
enum { _TK_OFFSET = 128,
       LIT_INT, TK_ID, LIT_STR, LIT_CHAR,
       TK_NE, /* != */ TK_EQ, /* == */
       TK_GE, /* >= */ TK_LE, /* <= */
       TK_INC, /* += */ TK_DEC, /* -= */
       TK_AND,/* && */ TK_OR, /* || */
       TK_LSHIFT, /* << */ TK_RSHIFT, /* >> */
       INT, CHAR, VOID,
       KW_BREAK, KW_CONTINUE, KW_DO, KW_ELSE, KW_ENUM,
       KW_FOR, KW_IF, KW_RETURN, KW_SIZEOF, KW_WHILE,
       C_PRINTF, C_EXIT,
       TK_COUNT };
// Identifier Kind
enum { UNDEFINED, GLOBAL, PARAM, LOCAL, FUNC, ENUM };
// Opcode
enum { OP_ADD = 128, OP_SUB, OP_MUL, OP_DIV, OP_REM,
       OP_MOV, OP_PUSH, OP_POP, OP_LOAD, OP_SAVE,
       OP_NE, OP_EQ, OP_GT, OP_GE, OP_LT, OP_LE,
       OP_RET,
       OP_JZ, OP_JUMP,
       OP_CALL,
       OP_PRINTF };
enum { EAX = 1, EBX, ECX, EDX, ESP, EBP, IMME };

struct Token {
    int kind;
    int value;
    int ln;
    char* start;
    char* end;
};

struct Symbol {
    // int datatype;
    int type;
    int tkIdx;
    int scope;
    int address; // offset to ebp
};

struct Ins {
    int op;
    int imme;
};

char *g_prog, *g_src;

// tokens
struct Token* g_tks;
int g_tkCnt, g_tkIter;

int g_regs[IMME];
struct Ins g_instructs[MAX_INS];
int g_insCnt;
int g_entry;
int g_pc; // program counter

char* ram;
int g_dataSize;

#define MAX_SCOPE 128
int g_scopeId;
int g_scopes[MAX_SCOPE];
int scopeCnt;

struct Symbol* syms;
int g_symCnt;

int expr();

// utility
void panic(char* fmt) {
    printf("[panic] %s\n", fmt);
    exit(1);
}

#ifndef NOT_DEVELOPMENT
#include "debug.inl"
#else
#define op2str(...)
#define reg2str(...)
#define dump_code(...)
#endif // #ifndef NOT_DEVELOPMENT

void lex() {
    int ln = 1;
    char *p = g_src;
    while (*p) {
        if (*p == '#' || (*p == '/' && *(p + 1) == '/')) {
            while (*p && *p != '\n') ++p;
            continue;
        }

        if (IS_WHITESPACE(*p)) {
            ln += (*p == '\n');
            p = p + 1;
            continue;
        }

        g_tks[g_tkCnt].ln = ln;
        g_tks[g_tkCnt].start = p;

        // id or keyword
        if (IS_LETTER(*p) || *p == '_') {
            g_tks[g_tkCnt].kind = TK_ID;
            for (++p; IS_LETTER(*p) || IS_DIGIT(*p) || *p == '_'; ++p);
            char *kw = "int char void "
                       "break continue do else enum for if return sizeof while "
                       "printf exit ";
            char *p0 = kw, *p1 = kw;
            for (int kind = INT; (p1 = strchr(p0, ' ')); p0 = p1 + 1, ++kind) {
                if (strncmp(p0, g_tks[g_tkCnt].start, p1 - p0) == 0) {
                    g_tks[g_tkCnt].kind = kind;
                    break;
                }
            }
            g_tks[g_tkCnt++].end = p;
            continue;
        }

        if (*p == '0' && p[1] == 'x') {
            g_tks[g_tkCnt].kind = LIT_INT;
            int result = 0;
            for (p += 2; IS_HEX(*p); ++p) {
                result = (result << 4) + ((*p < 'A') ? (*p - '0') : (*p - 55));
            }
            g_tks[g_tkCnt].value = result;
            g_tks[g_tkCnt++].end = p;
            continue;
        }

        if (IS_DIGIT(*p)) {
            g_tks[g_tkCnt].kind = LIT_INT;
            int result = 0;
            for (; IS_DIGIT(*p); ++p) {
                result = result * 10 + (*p - '0');
            }
            g_tks[g_tkCnt].value = result;
            g_tks[g_tkCnt++].end = p;
            continue;
        }

        // string
        if (*p == '"') {
            g_tks[g_tkCnt].kind = LIT_STR;
            for (++p; *p != '"'; ++p);
            g_tks[g_tkCnt++].end = ++p;
            continue;
        }

        // char
        if (*p == '\'') {
            /// TODO: escape
            g_tks[g_tkCnt].kind = LIT_CHAR;
            g_tks[g_tkCnt].value = p[1];
            g_tks[g_tkCnt++].end = (p += 3);
            continue;
        }

        g_tks[g_tkCnt].kind = *p;

        if (IS_PUNCT(p, '=', '=')) { g_tks[g_tkCnt].kind = TK_EQ; ++p; }
        else if (IS_PUNCT(p, '!', '=')) { g_tks[g_tkCnt].kind = TK_NE; ++p; }
        else if (IS_PUNCT(p, '>', '=')) { g_tks[g_tkCnt].kind = TK_GE; ++p; }
        else if (IS_PUNCT(p, '<', '=')) { g_tks[g_tkCnt].kind = TK_LE; ++p; }

        g_tks[g_tkCnt++].end = ++p;
    }
}

void enter_scope() {
    if (scopeCnt >= MAX_SCOPE) {
        panic("scope overflow");
    }

    g_scopes[scopeCnt++] = ++g_scopeId;
}

// debug
void debugprintsymbols() {
    DEVPRINT("********** symbol begin *************\n");
    for (int i = 0; i < g_symCnt; ++i) {
        int idx = syms[i].tkIdx;
        char* start = g_tks[idx].start;
        char* end = g_tks[idx].end;
        int len = end - start;

        DEVPRINT("[scope %d] %.*s", syms[i].scope, len, start);
        if (syms[i].type == FUNC) {
            DEVPRINT("(");
            for (int j = i + 1; syms[j].type == PARAM; ++j) {
                int idx = syms[j].tkIdx;
                char* start = g_tks[idx].start;
                char* end = g_tks[idx].end;
                int len = end - start;
                DEVPRINT("%.*s,", len, start);
            }
            DEVPRINT(") {}");
        } else if (syms[i].type == LOCAL) {
            // DEVPRINT("%d, scope %d\n", len, start, syms[i].address, syms[i].scope);
        }
        DEVPRINT("\n");
    }
    DEVPRINT("********** symbol end *************\n");
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
        ERROR("%d: expected token '%c'(%d), got '%.*s'\n",
            g_tks[g_tkIter].ln,
            kind < 128 ? kind : ' ',
            kind,
            g_tks[g_tkIter].end - g_tks[g_tkIter].start,
            g_tks[g_tkIter].start);
    }
    return g_tkIter++;
}

int expect_type() {
    int type = g_tks[g_tkIter].kind;
    if (IS_TYPE(type)) {
        ++g_tkIter;
        return type;
    } else {
        ERROR("%d: expected type specifier, got '%.*s'\n",
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
#define OP3(op, dest, src1) ((op) | (dest << 8) | (src1 << 16))
#define OP(op, dest, src1, src2) ((op) | (dest << 8) | (src1 << 16) | (src2 << 24))

struct CallToResolve {
    int insIdx;
    int tkIdx;
};

struct CallToResolve g_calls[256];
int g_callNum;

int post_expr() {
    int tkIdx = g_tkIter++;
    char* start = g_tks[tkIdx].start;
    char* end = g_tks[tkIdx].end;
    int ln = g_tks[tkIdx].ln;
    int kind = g_tks[tkIdx].kind;
    int len = end - start;
    int value = g_tks[tkIdx].value;
    if (kind == LIT_INT || kind == LIT_CHAR) {
        instruction(OP(OP_MOV, EAX, 0, IMME), value);
        return INT;
    }
    
    if (kind == LIT_STR) {
        if (g_dataSize > RAM_SIZE / 2)
            panic("data is running low");

        instruction(OP(OP_MOV, EAX, 0, IMME), (int)(ram + g_dataSize));
        for (int i = 1; i < len - 1; ++i) {
            int c = start[i];
            if (c == '\\') {
                c = start[++i];
                if (c == 'n') c = '\n';
                else panic("handle escape sequence");
            }
            ram[g_dataSize++] = c;
        }
        ram[g_dataSize++] = 0;
        g_dataSize = ALIGN(g_dataSize);
        return 0xFF0000 | CHAR; // char*
    }

    if (kind == '(') {
        int data_type = expr();
        expect(')');
        return data_type;
    }
    
    if (kind == TK_ID) {
        if (g_tks[g_tkIter].kind == '(') {
            ++g_tkIter;
            int argc;
            for (argc = 0; g_tks[g_tkIter].kind != ')'; ++argc) {
                if (argc > 0) expect(',');
                assign_expr();
                instruction(OP(OP_PUSH, 0, 0, EAX), 0);
            }

            g_calls[g_callNum].insIdx = g_insCnt;
            g_calls[g_callNum++].tkIdx = tkIdx;

            instruction(OP2(OP_CALL, 0), 0);
            if (argc) {
                instruction(OP(OP_ADD, ESP, ESP, IMME), argc << 2);
            }
            expect(')');
            return 0;
        }

        int address = 0;
        int type = UNDEFINED;
        for (int i = g_symCnt - 1; i >= 0; --i) {
            int tmp = syms[i].tkIdx;
            if (strncmp(g_tks[tmp].start, start, len) == 0) {
                address = syms[i].address;
                type = syms[i].type;
                break;
            }
        }

        if (type == GLOBAL) {
            panic("TODO: implement globlal variable");
        }

        if (type == UNDEFINED) {
            ERROR("%d: '%.*s' undeclared\n", ln, len, start);
        }

        instruction(OP(OP_SUB, EDX, EBP, IMME), address);
        instruction(OP(OP_LOAD, EAX, EDX, 0), 4);
        return 0;
    }

    if (kind == C_PRINTF) {
        expect('(');
        int argc = 0;
        for (; g_tks[g_tkIter].kind != ')'; ++argc) {
            if (argc > 0) expect(',');
            assign_expr();
            instruction(OP(OP_PUSH, 0, 0, EAX), 0);
        }
        if (argc > MAX_PRINF_ARGS) panic("printf supports at most %d args");
        instruction(OP(OP_SUB, ESP, ESP, IMME), (MAX_PRINF_ARGS - argc) << 2);
        instruction(OP_PRINTF, argc);
        instruction(OP(OP_ADD, ESP, ESP, IMME), MAX_PRINF_ARGS << 2);
        expect(')');
        return INT;
    }

    ERROR("%d: expected expression, got '%.*s'\n", ln, len, start);
    return INT;
}

int mul_expr() {
    post_expr();
    for (;;) {
        int optk = g_tks[g_tkIter].kind; int opcode;
        if (optk == '*') opcode = OP_MUL;
        else if (optk == '/') opcode = OP_DIV;
        else if (optk == '%') opcode = OP_REM;
        else break;
        ++g_tkIter;
        instruction(OP(OP_PUSH, 0, 0, EAX), 0);
        post_expr();
        instruction(OP2(OP_POP, EBX), 0);
        instruction(OP(opcode, EAX, EBX, EAX), 0);
    }

    return 0;
}

int add_expr() {
    /// TODO: data types
    mul_expr();
    for (;;) {
        int optk = g_tks[g_tkIter].kind; int opcode;
        if (optk == '+') opcode = OP_ADD;
        else if (optk == '-') opcode = OP_SUB;
        else break;
        ++g_tkIter;
        instruction(OP(OP_PUSH, 0, 0, EAX), 0);
        mul_expr();
        instruction(OP2(OP_POP, EBX), 0);
        instruction(OP(opcode, EAX, EBX, EAX), 0);
    }

    return 0;
}

int relation_expr() {
    add_expr();
    for (;;) {
        int kind = g_tks[g_tkIter].kind; int opcode;
        if (kind == TK_NE) opcode = OP_NE;
        else if (kind == TK_EQ) opcode = OP_EQ;
        else if (kind == '<') opcode = OP_LT;
        else if (kind == '>') opcode = OP_GT;
        else if (kind == TK_GE) opcode = OP_GE;
        else if (kind == '<') opcode = OP_LT;
        else if (kind == TK_LE) opcode = OP_LE;
        else break;
        ++g_tkIter;
        instruction(OP(OP_PUSH, 0, 0, EAX), 0);
        add_expr();
        instruction(OP2(OP_POP, EBX), 0);
        instruction(OP(opcode, EAX, EBX, EAX), 0);
    }
    return INT;
}

int assign_expr() {
    relation_expr();
    for (;;) {
        int kind = g_tks[g_tkIter].kind;
        if (kind == '=') {
            ++g_tkIter;
            instruction(OP(OP_PUSH, 0, 0, EDX), 0);
            int rhs = relation_expr();
            instruction(OP2(OP_POP, EDX), 0);
            if (rhs == CHAR) {
                panic("TODO: implement load char");
            } else {
                instruction(OP(OP_SAVE, EDX, EAX, 0), 4);
            }
        }
        else break;
    }

    return INT;
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
    if (kind == KW_RETURN) {
        if (g_tks[++g_tkIter].kind != ';') assign_expr();
        instruction(OP(OP_MOV, ESP, 0, EBP), 0);
        instruction(OP2(OP_POP, EBP), 0);
        instruction(OP_RET, 0);
        expect(';');
        return;
    }

    if (kind == KW_IF) {
        //     eax == 0; goto L1 |     eax == 0; goto L1
        //     ...               |     ...
        //     goto L2           | L1: ...
        // L1: ...               |
        // L2: ...               |
        ++g_tkIter;
        expect('('); expr(); expect(')');
        int goto_L1 = g_insCnt;
        instruction(OP_JZ, 0);
        stmt();

        if (g_tks[g_tkIter].kind != KW_ELSE) {
            g_instructs[goto_L1].imme = g_insCnt;
            return;
        }

        ++g_tkIter; // skip else
        int goto_L2 = g_insCnt;
        instruction(OP_JUMP, g_insCnt + 1);
        g_instructs[goto_L1].imme = g_insCnt;
        stmt();
        g_instructs[goto_L2].imme = g_insCnt;
        return;
    }

    if (kind == KW_WHILE) {
        // TEST: ...
        //       eax == 0; goto END
        //       ...
        //       jump TEST;
        // END:
        int test_label = g_insCnt;
        ++g_tkIter;
        expect('('); expr(); expect(')');
        int goto_end = g_insCnt;
        instruction(OP_JZ, 0);
        stmt();
        instruction(OP_JUMP, test_label);
        g_instructs[goto_end].imme = g_insCnt;
        return;
    }

    if (kind == KW_DO) {
        // START: ...
        //        eax == 0; goto END
        //        jump START;
        // EMD:   ...
        int start_label = g_insCnt;
        ++g_tkIter;
        stmt();
        expect(KW_WHILE);
        expect('('); expr(); expect(')');
        int goto_end = g_insCnt;
        instruction(OP_JZ, 0);
        instruction(OP_JUMP, start_label);
        g_instructs[goto_end].imme = g_insCnt;
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

                    int id = expect(TK_ID);
                    char* start = g_tks[id].start;
                    int len = g_tks[id].end - start;
                    int prev = g_symCnt - 1;
                    syms[g_symCnt].address = 4;

                    if (prev >= 0 && syms[prev].type == LOCAL) {
                        syms[g_symCnt].address += syms[prev].address;
                    }

                    syms[g_symCnt].type = LOCAL;
                    syms[g_symCnt].tkIdx = id;
                    syms[g_symCnt].scope = g_scopes[scopeCnt - 1];
                    ++g_symCnt;
                    // ++g_tkIter;

                    instruction(OP(OP_SUB, ESP, ESP, IMME), 4);
                    if (g_tks[g_tkIter].kind == '=') {
                        ++g_tkIter;
                        assign_expr();
                        instruction(OP(OP_SAVE, ESP, EAX, 0), 4);
                    }

                    ++restore, ++varNum;
                } while (g_tks[g_tkIter].kind != ';');

                ++g_tkIter;
            } else {
                stmt();
            }
        }
        ++g_tkIter;

        if (restore) instruction(OP(OP_ADD, ESP, ESP, IMME), restore << 2);
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
    expect_type();
    int id = expect(TK_ID);

    if (g_tks[g_tkIter].kind == '(') {
        if (strncmp("main", g_tks[id].start, 4) == 0) {
            g_entry = g_insCnt;
        } else {
            syms[g_symCnt].type = FUNC;
            syms[g_symCnt].tkIdx = id;
            syms[g_symCnt++].address = g_insCnt;
        }

        enter_scope();
        expect('(');
        int argCnt = 0;
        while (g_tks[g_tkIter].kind != ')') {
            if (argCnt > 0) {
                expect(',');
            }

            int type = expect_type();
            int ptr = 0;
            for (; g_tks[g_tkIter].kind == '*'; ++g_tkIter) { ptr = (ptr << 8) | 0xFF; }
            type = (ptr << 16) | type;

            syms[g_symCnt].tkIdx = expect(TK_ID);
            syms[g_symCnt].scope = g_scopes[scopeCnt - 1];
            syms[g_symCnt++].type = PARAM;
            ++argCnt;
        }
        expect(')');
        for (int i = 1; i <= argCnt; i = i + 1) {
            syms[g_symCnt - i].address = -((i + 1) << 2);
        }

        // save frame
        instruction(OP(OP_PUSH, 0, 0, EBP), 0);
        instruction(OP(OP_MOV, EBP, 0, ESP), 0);
        stmt();
        exit_scope();
        return;
    }

    panic("TODO: implement global variable/enum");
}

void gen(int argc, char** argv) {
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
        int len = end - start;

        int found = 0;
        for (int j = 0; j < g_symCnt; ++j) {
            if (syms[j].type == FUNC) {
                int funcIdx = syms[j].tkIdx;
                if (strncmp(start, g_tks[funcIdx].start, len) == 0) {
                    found = 1;
                    g_instructs[g_calls[i].insIdx].imme = syms[j].address;
                    break;
                }
            }
        }

        if (!found) {
            ERROR("unknown reference to call %.*s\n", len, start);
        }

    }

    exit_scope();

    // start
    int entry = g_insCnt;
    instruction(OP(OP_PUSH, 0, 0, IMME), argc);
    instruction(OP(OP_PUSH, 0, 0, IMME), (int)argv);
    instruction(OP2(OP_CALL, 0), g_entry);

    g_entry = entry;
}

void printvm() {
    DEVPRINT("********** VM begin **********\n");
    DEVPRINT("data  [0x%p - 0x%p]\n", ram, ram + g_dataSize);
    DEVPRINT("stack [0x%p - 0x%p]\n", ram + g_dataSize, ram + RAM_SIZE);
    DEVPRINT("eax: %d\n", g_regs[EAX]);
    DEVPRINT("ebx: %d\n", g_regs[EBX]);
    DEVPRINT("ecx: %d\n", g_regs[ECX]);
    DEVPRINT("edx: 0x%X\n", g_regs[EDX]);
    DEVPRINT("esp: 0x%X\n", g_regs[ESP]);
    DEVPRINT("ebp: 0x%X\n", g_regs[EBP]);
    DEVPRINT("pc: %d\n", g_entry);
    DEVPRINT("********** VM end **********\n");
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

        if (op == OP_CALL) {
            g_regs[ESP] -= 4;
            ((int*)ram)[g_regs[ESP] >> 2] = pc + 1;
            pc = imme;
            continue;
        }

        if ((op == OP_JUMP)) {
            pc = imme;
            continue;
        }

        if (op == OP_JZ) {
            if (g_regs[EAX] == 0) pc = imme;
            else pc = pc + 1;
            continue;
        }

        if (op == OP_RET) {
            pc = ((int*)ram)[g_regs[ESP] >> 2];
            g_regs[ESP] += 4;
            continue;
        }

        pc = pc + 1;

        if (op == OP_MOV) {
            g_regs[dest] = value;
        } else if (op == OP_PUSH) {
            g_regs[ESP] -= 4;
            ((int*)ram)[g_regs[ESP] >> 2] = value;
        } else if (op == OP_POP) {
            g_regs[dest] = ((int*)ram)[g_regs[ESP] >> 2];
            g_regs[ESP] += 4;
        } else if (op == OP_ADD) {
            g_regs[dest] = g_regs[src1] + value;
        } else if (op == OP_SUB) {
            g_regs[dest] = g_regs[src1] - value;
        } else if (op == OP_MUL) {
            g_regs[dest] = g_regs[src1] * value;
        } else if (op == OP_DIV) {
            g_regs[dest] = g_regs[src1] / value;
        } else if (op == OP_REM) {
            g_regs[dest] = g_regs[src1] % value;
        } else if (op == OP_EQ) {
            g_regs[dest] = g_regs[src1] == value;
        } else if (op == OP_NE) {
            g_regs[dest] = g_regs[src1] != value;
        } else if (op == OP_GE) {
            g_regs[dest] = g_regs[src1] >= value;
        } else if (op == OP_GT) {
            g_regs[dest] = g_regs[src1] > value;
        } else if (op == OP_LE) {
            g_regs[dest] = g_regs[src1] <= value;
        } else if (op == OP_LT) {
            g_regs[dest] = g_regs[src1] < value;
        } else if (op == OP_SAVE) {
            if (imme != 4) {
                panic("TODO: implement save byte");
            }
            ((int*)ram)[g_regs[dest] >> 2] = g_regs[src1];
        } else if (op == OP_LOAD) {
            if (imme != 4) {
                panic("TODO: implement load byte");
            }
            g_regs[dest] = ((int*)ram)[g_regs[src1] >> 2];
        } else if (op == OP_PRINTF) {
            int slot = g_regs[ESP] >> 2;
            printf((char*)((int*)ram)[slot + 7],
                   ((int*)ram)[slot + 6],
                   ((int*)ram)[slot + 5],
                   ((int*)ram)[slot + 4],
                   ((int*)ram)[slot + 3],
                   ((int*)ram)[slot + 2],
                   ((int*)ram)[slot + 1],
                   ((int*)ram)[slot + 0]);
        } else {
            panic("Invalid op code");
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
        char* names = "INT   ID    STR   CHAR  NE    EQ    LE    GE    "
                      "INC   DEC   AND   OR    LSHIFTRSHIFT"
                      "Int   Char  Void  "
                      "Break Cont  Do    Else  Enum  For   If    Ret   SizeofWhile "
                      "PrintfExit  ";

        printf("%.*s", len, start);
        if (kind > _TK_OFFSET) {
            char *p = names + 6 * (kind - _TK_OFFSET - 1);
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

int main(int argc, char **argv) {
    g_prog = argv[0];

    if (argc < 2) {
        printf("Usage: %s file [args...]\n", g_prog);
        exit(1);
    }

    // initialization
    ram = calloc(1, RAM_SIZE);
    g_regs[ESP] = RAM_SIZE;

    // store source to buffer
    void* fp = fopen(argv[1], "r");
    if (!fp) {
        printf("file '%s' does not exist\n", argv[1]);
        return 1;
    }

    fseek(fp, 0, 2); // SEEK_END
    int len = ftell(fp);
    fseek(fp, 0, 0); // SEEK_SET
    g_src = calloc(1, len + 1); // pad
    fread(g_src, 1, len, fp);
    fclose(fp);

    g_tks = calloc(len, sizeof(struct Token));
    syms = calloc(len, sizeof(struct Symbol));

    int i = len * (1 + sizeof(struct Token) + sizeof(struct Symbol));
    DEVPRINT("allocate %d kb\n", i / 1024);

    lex();
    DEVPRINT("-------- lex --------\n");
#if !defined(NOT_DEVELOPMENT) && !defined(TEST)
    dump_tokens();
#endif

    gen(argc - 1, argv + 1);
    DEVPRINT("-------- code --------\n");
    dump_code();

    DEVPRINT("-------- vm --------\n");
    printvm();
    DEVPRINT("-------- exec --------\n");
    exec();

    free(ram);
    free(g_tks);
    free(g_src);

    DEVPRINT("-------- exiting --------\n");
    DEVPRINT("script '%s' exit with code %d\n", argv[1], g_regs[EAX]);
    return 0;
}
