#ifndef NOT_DEVELOPMENT
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#endif // #ifndef NOT_DEVELOPMENT

//#define switch @
//#define case @
//#define default @

// |  op  |  dst  |  src1  |  src2  |
// | 0-7  |  8-15 | 16-24  | 25-31  |
//- instruction sets (src2 is either register or immediate value)
//-
//- push src2           >- esp -= 4; [esp] = src2
//- pop dest            >- dest = [esp]; esp += 4
//- mov dest, src2      >- dest = src2
//-
//- add:    dst = x + y
//- sub:    dst = x - y
//- mul:    dst = x * y
//- div:    dst = x / y
//- rem:    dst = x % y
//- store:  store [x] y
//- load:   load x [y]
//- loadc:  load x 0xFF & [y]
//- ret:    ret
//- jz:     if eax == 0; jump pc
//- jump:   jump imme
//- printf: call c printf, push 8 args onto stack

#if defined(TEST) || defined(NOT_DEVELOPMENT)
#define DEVPRINT(...)
#else
#define DEVPRINT(...) fprintf(stderr, __VA_ARGS__)
#endif

#define ERROR(...) { printf("\n%s:", g_prog); printf(__VA_ARGS__); exit(1); }

// definitions
enum { TK_INVALID = 0,
       TK_LP = '(', TK_RP = ')',
       TK_LB = '{', TK_RB = '}',
       TK_LS = '[', TK_RS = ']',
       TK_SC = ';', TK_COMMA = ',',
       TK_PLUS = '+', TK_MINUS = '-', TK_STAR = '*', TK_SLASH = '/', TK_REM = '%',
       TK_NOT = '!',
       TK_ASSIGN = '=',
       TL_GT = '>',
       TK_LT = '<',
       _OFFSET_1 = 128,
       TK_NE, /* != */
       TK_EQ, /* == */
       TK_GE, /* >= */
       TK_LE, /* <= */
       TK_INT, TK_ID, TK_STR, TK_CHAR,
       _KEYWORD_OFFSET,
       KW_INT, KW_IF, KW_ELSE, KW_RET, KW_PRINTF };
enum { T_VOID, T_INT, T_CHAR, T_PTR };
enum { UNDEFINED, GLOBAL, PARAM, LOCAL, FUNC, ENUM };
enum { OP_ADD = 128, OP_SUB, OP_MUL, OP_DIV, OP_REM,
       OP_MOV, OP_PUSH, OP_POP, OP_STORE, OP_LOAD, OP_LOADBYTE,
       OP_NE, OP_EQ, OP_GT, OP_GE, OP_LT, OP_LE,
       OP_RET,
       OP_JZ, OP_JUMP,
       OP_CALL,
       OP_PRINTF };
enum { EAX = 1, EBX, ECX, EDX, ESP, EBP, IMME };

struct Token {
    int kind;
    int ln;
    char* start;
    char* end;
};

struct Symbol {
    int type;
    int tkIdx;
    int scope;
    int address; // offset to ebp
};

struct Ins {
    int op;
    int imme;
};

// globals
#define RAM_SIZE (1 << 20)
#define MAX_SYMBOL (1 << 8)
#define MAX_INS (1 << 12)
#define MAX_PRINF_ARGS 8

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

struct Symbol syms[MAX_SYMBOL];
int g_symCnt;

void expr();

// utility
void panic(char* fmt) {
    printf("[panic] %s\n", fmt);
    exit(1);
}

int str2int(char* p, char* end) {
    int result = 0;
    for (; p != end; ++p) {
        result = result * 10 + (*p - '0');
    }
    return result;
}

// TODO: remove this
char* tk2str(int kd) {
    switch (kd) {
        case TK_INVALID: return "ERR";
        case TK_INT: return "INT";
        case TK_ID: return "ID";
        case TK_STR: return "STR";
        case TK_CHAR: return "CHAR";
        case TK_LP: return "'('";
        case TK_RP: return "')'";
        case TK_LB: return "'{'";
        case TK_RB: return "'}'";
        case TK_LS: return "'['";
        case TK_RS: return "']'";
        case TK_COMMA: return "','";
        case TK_SC: return "';'";
        case TK_ASSIGN: return "assign";
        case TK_PLUS: return "add";
        case TK_MINUS: return "sub";
        case TK_STAR: return "mul";
        case TK_SLASH: return "div";
        case TK_REM: return "rem";
        case TK_EQ: return "EQ";
        case TK_NE: return "NE";
        case TK_GE: return "GE";
        case TL_GT: return "GT";
        case TK_LE: return "LE";
        case TK_LT: return "LT";
        case KW_INT: return "Int";
        case KW_RET: return "Ret";
        case KW_IF: return "If";
        case KW_ELSE: return "Else";
        case KW_PRINTF: return "Print";
        default: printf("[%d]", kd); panic("unknown token"); return "<error>";
    }
}

#ifndef NOT_DEVELOPMENT
#include "debug.inl"
#else
#define op2str(...)
#define reg2str(...)
#define dump_tk(...)
#define dump_tks(...)
#define dump_code(...)
#endif // #ifndef NOT_DEVELOPMENT

/// TODO: && elimintaion
#define IS_LETTER(C) ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z'))
#define IS_DIGIT(C) (C >= '0' && C <= '9')
#define IS_WHITESPACE(C) (C == ' ' || C == '\n' || C == '\r' || C == '\t')

void lex() {
    int ln = 1;
    char *p = g_src, *p0, *p1;
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
            char* g_kw = "int if else return printf ";
            p0 = g_kw, p1 = g_kw;
            for (int offset = 1; (p1 = strchr(p0, ' ')); p0 = p1 + 1, ++offset) {
                if (strncmp(p0, g_tks[g_tkCnt].start, p1 - p0) == 0) {
                    g_tks[g_tkCnt].kind = _KEYWORD_OFFSET + offset;
                    break;
                }
            }
            g_tks[g_tkCnt++].end = p;
            continue;
        }

        // integer
        if (IS_DIGIT(*p)) {
            g_tks[g_tkCnt].kind = TK_INT;
            for (++p; IS_DIGIT(*p); ++p);
            g_tks[g_tkCnt++].end = p;
            continue;
        }

        /// TODO: hex

        // string
        if (*p == '"') {
            g_tks[g_tkCnt].kind = TK_STR;
            for (++p; *p != '"'; ++p);
            g_tks[g_tkCnt++].end = ++p;
            continue;
        }

        // TODO: refactor equality
        if (*p == '=' && *(p + 1) == '=') {
            g_tks[g_tkCnt].kind = TK_EQ;
            g_tks[g_tkCnt++].end = (p += 2);
            continue;
        }
        if (*p == '!' && *(p + 1) == '=') {
            g_tks[g_tkCnt].kind = TK_NE;
            g_tks[g_tkCnt++].end = (p += 2);
            continue;
        }
        if (*p == '>' && *(p + 1) == '=') {
            g_tks[g_tkCnt].kind = TK_GE;
            g_tks[g_tkCnt++].end = (p += 2);
            continue;
        }
        if (*p == '<' && *(p + 1) == '=') {
            g_tks[g_tkCnt].kind = TK_LE;
            g_tks[g_tkCnt++].end = (p += 2);
            continue;
        }
        if ((p0 = strchr("()[]{},;+-*/%=><", *p))) {
            g_tks[g_tkCnt].kind = *p0;
            g_tks[g_tkCnt++].end = ++p;
            continue;
        }

        ERROR("%d: strayed char '%c'\n", ln, *p);
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
        ERROR("%d: expected %s, got '%.*s'\n",
            g_tks[g_tkIter].ln,
            tk2str(kind),
            g_tks[g_tkIter].end - g_tks[g_tkIter].start,
            g_tks[g_tkIter].start);
    }
    return g_tkIter++;
}

int expect_type() {
    int type = g_tks[g_tkIter].kind;
    if (type == KW_INT) {
        ++g_tkIter;
        return T_INT;
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

void expr_post() {
    int tkIdx = g_tkIter++;
    char* start = g_tks[tkIdx].start;
    char* end = g_tks[tkIdx].end;
    int ln = g_tks[tkIdx].ln;
    int kind = g_tks[tkIdx].kind;
    int len = end - start;
    if (kind == TK_INT) {
        int val = str2int(start, end);
        instruction(OP(OP_MOV, EAX, 0, IMME), val);
    } else if (kind == TK_STR) {
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
        // 4 byte aligned
        g_dataSize = (g_dataSize + 3) & (0xFFFFFFFF << 2);
    } else if (kind == TK_LP) {
        expr();
        expect(TK_RP);
    } else if (kind == TK_ID) {
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

        if (type == UNDEFINED) {
            ERROR("%d: '%.*s' undeclared\n", ln, len, start);
        }

        if (type == FUNC) {
            expect(TK_LP);
            int argc;
            for (argc = 0; g_tks[g_tkIter].kind != TK_RP; ++argc) {
                if (argc > 0) expect(TK_COMMA);
                expr();
                instruction(OP(OP_PUSH, 0, 0, EAX), 0);
            }

            g_calls[g_callNum].insIdx = g_insCnt;
            g_calls[g_callNum++].tkIdx = tkIdx;

            instruction(OP2(OP_CALL, 0), 0);
            if (argc) {
                instruction(OP(OP_ADD, ESP, ESP, IMME), argc << 2);
            }
            expect(TK_RP);
        } else if (type == GLOBAL) {
            panic("TODO: implement globlal variable");
        } else {
            instruction(OP(OP_SUB, EDX, EBP, IMME), address);
            instruction(OP(OP_LOAD, EAX, EDX, 0), 0);
        }
    } else if (kind == KW_PRINTF) {
        expect(TK_LP);
        int argc = 0;
        for (; g_tks[g_tkIter].kind != TK_RP; ++argc) {
            if (argc > 0) expect(TK_COMMA);
            expr();
            instruction(OP(OP_PUSH, 0, 0, EAX), 0);
        }
        if (argc > MAX_PRINF_ARGS) panic("printf supports at most %d args");
        instruction(OP(OP_SUB, ESP, ESP, IMME), (MAX_PRINF_ARGS - argc) << 2);
        instruction(OP_PRINTF, argc);
        instruction(OP(OP_ADD, ESP, ESP, IMME), MAX_PRINF_ARGS << 2);
        expect(TK_RP);
    } else {
        ERROR("%d: expected expression, got '%.*s'\n", ln, len, start);
    }
}

void expr_mul() {
    expr_post();
    for (;;) {
        int optk = g_tks[g_tkIter].kind; int opcode;
        if (optk == TK_STAR) opcode = OP_MUL;
        else if (optk == TK_SLASH) opcode = OP_DIV;
        else if (optk == TK_REM) opcode = OP_REM;
        else break;
        ++g_tkIter;
        instruction(OP(OP_PUSH, 0, 0, EAX), 0);
        expr_post();
        instruction(OP2(OP_POP, EBX), 0);
        instruction(OP(opcode, EAX, EBX, EAX), 0);
    }
}

void expr_add() {
    expr_mul();
    for (;;) {
        int optk = g_tks[g_tkIter].kind; int opcode;
        if (optk == TK_PLUS) opcode = OP_ADD;
        else if (optk == TK_MINUS) opcode = OP_SUB;
        else break;
        ++g_tkIter;
        instruction(OP(OP_PUSH, 0, 0, EAX), 0);
        expr_mul();
        instruction(OP2(OP_POP, EBX), 0);
        instruction(OP(opcode, EAX, EBX, EAX), 0);
    }
}

void expr_equal() {
    expr_add();
    for (;;) {
        int optk = g_tks[g_tkIter].kind; int opcode;
        if (optk == TK_NE) opcode = OP_NE;
        else if (optk == TK_EQ) opcode = OP_EQ;
        else if (optk == TK_LT) opcode = OP_LT;
        else if (optk == TL_GT) opcode = OP_GT;
        else if (optk == TK_GE) opcode = OP_GE;
        else if (optk == TK_LT) opcode = OP_LT;
        else if (optk == TK_LE) opcode = OP_LE;
        else break;
        ++g_tkIter;
        instruction(OP(OP_PUSH, 0, 0, EAX), 0);
        expr_add();
        instruction(OP2(OP_POP, EBX), 0);
        instruction(OP(opcode, EAX, EBX, EAX), 0);
    }
}

void expr() {
    expr_equal();
}

void stmt() {
    int kind = g_tks[g_tkIter].kind;
    if (kind == KW_RET) {
        if (g_tks[++g_tkIter].kind != TK_SC) expr();
        instruction(OP(OP_MOV, ESP, 0, EBP), 0);
        instruction(OP2(OP_POP, EBP), 0);
        instruction(OP_RET, 0);
        expect(TK_SC);
    } else if (kind == KW_IF) {
        // if eax == 0, jmp L1
        // ...
        // jump L2:
        // L1: ...
        // L2: ...
        ++g_tkIter;
        expect(TK_LP);
        expr();
        expect(TK_RP);
        int jump1Loc = g_insCnt;
        instruction(OP_JZ, 0);
        stmt();
        int jump2Loc = g_insCnt;
        instruction(OP_JUMP, g_insCnt + 1);
        g_instructs[jump1Loc].imme = g_insCnt;
        if (g_tks[g_tkIter].kind == KW_ELSE) {
            ++g_tkIter; // skip else
            stmt();
            g_instructs[jump2Loc].imme = g_insCnt;
        }
    } else if (kind == TK_LB) {
        enter_scope();
        ++g_tkIter;
        int restore = 0;
        while (g_tks[g_tkIter].kind != TK_RB) {
            kind = g_tks[g_tkIter].kind;
            if (kind == KW_INT /*TODO: Char, Ptr*/) {
                expect_type();
                instruction(OP(OP_SUB, ESP, ESP, IMME), 4);
                char* start = g_tks[g_tkIter].start;
                int len = g_tks[g_tkIter].end - start;
                for (int i = g_symCnt - 1; syms[i].scope == g_scopes[scopeCnt - 1]; --i) {
                    int tmpId = syms[i].tkIdx;
                    if (strncmp(g_tks[tmpId].start, start, len) == 0) {
                        ERROR(
                            "%d: redeclaration of '%.*s', previously defined on line %d\n",
                            g_tks[g_tkIter].ln, len, start, g_tks[tmpId].ln);
                    }
                }

                if (g_symCnt >= MAX_SYMBOL) {
                    panic("symbol overflow");
                }

                int prev = g_symCnt - 1;
                syms[g_symCnt].address = 4;
                if (prev >= 0 && syms[prev].type == LOCAL) {
                    syms[g_symCnt].address += syms[prev].address;
                }

                syms[g_symCnt].type = LOCAL;
                syms[g_symCnt].tkIdx = g_tkIter;
                syms[g_symCnt].scope = g_scopes[scopeCnt - 1];
                ++g_symCnt;
                ++g_tkIter;

                if (g_tks[g_tkIter].kind == TK_ASSIGN) {
                    ++g_tkIter;
                    expr();
                    instruction(OP(OP_STORE, ESP, EAX, 0), 0);
                }

                ++restore;
                expect(TK_SC);
            } else {
                stmt();
            }
        }
        ++g_tkIter;

        if (restore) instruction(OP(OP_ADD, ESP, ESP, IMME), restore << 2);
        exit_scope();
    } else if (kind == TK_SC) {
        ++g_tkIter;
    } else {
        expr();
        expect(TK_SC);
    }
}

// an object could be a global variable, an enum or a function
void obj() {
    // type
    expect_type();
    int id = expect(TK_ID);

    if (g_tks[g_tkIter].kind == TK_LP) {
        if (strncmp("main", g_tks[id].start, 4) == 0) {
            g_entry = g_insCnt;
        } else {
            syms[g_symCnt].type = FUNC;
            syms[g_symCnt].tkIdx = id;
            syms[g_symCnt++].address = g_insCnt;
        }

        enter_scope();
        expect(TK_LP);
        int argCnt = 0;
        while (g_tks[g_tkIter].kind != TK_RP) {
            if (argCnt > 0) {
                expect(TK_COMMA);
            }

            expect_type();
            syms[g_symCnt].tkIdx = expect(TK_ID);
            syms[g_symCnt].scope = g_scopes[scopeCnt - 1];
            syms[g_symCnt++].type = PARAM;
            ++argCnt;
        }
        expect(TK_RP);
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
        } else if (op == OP_STORE) {
            ((int*)ram)[g_regs[dest] >> 2] = g_regs[src1];
        } else if (op == OP_LOAD) {
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

    lex();
    DEVPRINT("-------- lex --------\n");
    dump_tks();

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
