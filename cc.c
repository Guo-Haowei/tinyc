#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// no char array[] on stack

#if !defined(_TEST)
#define DEVPRINT(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEVPRINT(...)
#endif

// definitions
enum { TkErr = 0,
       TkLP = '(', TkRP = ')',
       TkLB = '{', TkRB = '}',
       TkLS = '[', TkRS = ']',
       TkSC = ';', TkComma = ',',
       TkAdd = '+', TkSub = '-', TkMul = '*', TkDiv = '/', TkRem = '%',
       TkEq = '=',
       TkInt = 128, TkId, TkStr, TkChar,
       _KwOffset, KwInt, KwRet, KwPrintf };

enum { TVoid, TInt, TChar, TPtr };

enum { Global };

// vm
enum { _Offset = 1, OpAdd, OpSub, OpMul, OpDiv, OpRem,
       OpPush, OpPop, OpMov, OpSave, OpLoad, OpRet,
       CPrintf };
enum { RegEax = 1, RegEbx, RegEcx, RegEdx, RegEsp, RegEbp, Imme };

struct Token {
    int kind;
    int ln;
    char* start;
    char* end;
};

// data type
struct DataType {
    int kind;
};

struct Symbol {
    int tkIdx;
    int scope; // Global or Local
    int address; // offset to ebp
};

// |  op  |  dst  |  src1  |  src2  |
// | 0-7  |  8-15 | 16-24  | 25-31  |
struct Ins {
    int op;
    int imme;
};

// globals
// source code
#define MAX_SRC (1 << 12)
#define MAX_TOKEN (1 << 10)
#define MAX_SYMBOL (1 << 8)
#define MAX_INS (1 << 12)
#define STACK_SIZE (1 << 16)
#define DATA_SECTION_SIZE (1 << 12)
#define INT_SIZE 4
#define MAX_PRINF_ARGS 8

char src[MAX_SRC]; char *p; int ln;

// tokens
struct Token tks[MAX_TOKEN]; int tkNum; int tkIter;

int regs[Imme];
struct Ins ins[MAX_INS];
int insNum;

char* stack;
char* data;
int ds;

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

// debug only
#include "debug.inl"
// debug only

void lex() {
    char* pp;
    while (*p) {
        if (tkNum >= MAX_TOKEN) {
            panic("tks overflow\n");
        }

        // skip comment or preproc
        if (*p == '#' || (*p == '/' && *(p + 1) == '/')) {
            while (*p && *p != '\n') ++p;
        // line ending
        } else if (*p == '\n') {
            ++ln; ++p;
        // whitespace
        } else if (*p == ' ' || *p == '\r' || *p == '\t') {
            ++p;
        // id or keyword
        } else if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || *p == '_') {
            tks[tkNum].kind = TkId; tks[tkNum].ln = ln; tks[tkNum].start = p;
            for (++p; (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') || *p == '_'; ++p);
            // check if keyword
            char* kw = "int return printf "; pp = kw;
            for (int offset = 1; (pp = strchr(kw, ' ')); kw = pp + 1, ++offset) {
                if (strncmp(kw, tks[tkNum].start, pp - kw) == 0) {
                    tks[tkNum].kind = _KwOffset + offset;
                    break;
                }
            }
            tks[tkNum++].end = p;
        // num
        } else if (*p >= '0' && *p <= '9') {
            tks[tkNum].kind = TkInt; tks[tkNum].ln = ln; tks[tkNum].start = p;
            for (++p; (*p >= '0' && *p <= '9'); ++p);
            tks[tkNum++].end = p;
        // string
        } else if (*p == '"') {
            tks[tkNum].kind = TkStr; tks[tkNum].ln = ln; tks[tkNum].start = p;
            for (++p; *p != '"'; ++p);
            tks[tkNum++].end = ++p;
        // punct
        } else if ((pp = strchr("()[]{},;+-*/%=", *p))) {
            tks[tkNum].kind = *pp; tks[tkNum].ln = ln; tks[tkNum].start = p;
            tks[tkNum++].end = ++p;
        } else {
            printf("error: strayed char %c on line %d\n", *p, ln);
            exit(1);
        }
    }
}

#define MAX_SCOPE 128
int scopeId;
int scopeIter;
int scope[MAX_SCOPE];
struct Symbol syms[MAX_SYMBOL];
int symCnt;

void enter_scope() {
    if (scopeIter >= MAX_SCOPE) {
        panic("scope overflow");
    }

    ++scopeId;
    scope[scopeIter++] = scopeId;
}

void exit_scope() {
    if (scopeIter <= 0) {
        panic("scope overflow");
    }

    --scopeIter;
    // TODO: clean up symbols in this scope
}

int expect(int kind) {
    if (tks[tkIter].kind != kind) {
        printf("expect %s on line %d, got %.*s\n", tk2str(kind), tks[tkIter].ln, tks[tkIter].end - tks[tkIter].start, tks[tkIter].start);
        exit(1);
    }
    return tkIter++;
}

void expect_type(int* kind, int* offset) {
    int type = tks[tkIter].kind;
    if (type == KwInt) {
        *kind = TInt;
        *offset = 4;
        ++tkIter;
    } else {
        printf("expect type specifier on line %d, got %.*s\n", tks[tkIter].ln, tks[tkIter].end - tks[tkIter].start, tks[tkIter].start);
        exit(1);
    }
}

void add_ins(int op, int imme) {
    if (insNum >= MAX_INS) {
        panic("instruction overflow");
    }

    ins[insNum].op = op;
    ins[insNum].imme = imme;
    ++insNum;
}

#define OP(op, dest, src1, src2) ((op) | (dest << 8) | (src1 << 16) | (src2 << 24))

void expr_atomic() {
    int kind = tks[tkIter].kind;
    if (kind == TkInt) {
        int val = str2int(tks[tkIter].start, tks[tkIter].end);
        add_ins(OP(OpMov, RegEax, 0, Imme), val);
        ++tkIter;
    } else if (kind == TkStr) {
        if (ds > DATA_SECTION_SIZE / 2)
            panic("data is running low");

        char* p = tks[tkIter].start;
        int len = tks[tkIter].end - tks[tkIter].start;
        add_ins(OP(OpMov, RegEax, 0, Imme), (int)(data + ds));
        for (int i = 1; i < len - 1; ++i) {
            int c = p[i];
            if (c == '\\') {
                c = p[++i];
                if (c == 'n') c = '\n';
                else panic("handle escape sequence");
            }
            data[ds++] = c;
        }
        data[ds++] = 0;
        ds = (ds + 3) & -3; // align data
        ++tkIter;
    } else if (kind == TkLP) {
        ++tkIter;
        expr();
        expect(TkRP);
    } else {
        printf("expect expression, got %.*s\n", tks[tkIter].end - tks[tkIter].start, tks[tkIter].start);
        exit(1);
    }
}

void expr_post() {
    int id = tkIter;
    int kind = tks[tkIter].kind;
    if (kind == TkId) {
        panic("TODO: implement func call");
    } else if (kind == KwPrintf) {
        ++tkIter;
        expect(TkLP);
        int argc = 0;
        for (; tks[tkIter].kind != TkRP; ++argc) {
            if (argc > 0) expect(TkComma);
            expr();
            add_ins(OP(OpPush, RegEax, 0, 0), 0);
        }
        if (argc > MAX_PRINF_ARGS) panic("printf supports at most %d args");
        add_ins(OP(OpSub, RegEsp, RegEsp, Imme), (MAX_PRINF_ARGS - argc) << 2);
        add_ins(OP(CPrintf, 0, 0, 0), argc);
        add_ins(OP(OpAdd, RegEsp, RegEsp, Imme), MAX_PRINF_ARGS << 2);
        expect(TkRP);
    } else {
        expr_atomic();
    }
}

void expr_mul() {
    expr_post();
    for (;;) {
        int optk = tks[tkIter].kind; int opcode;
        if (optk == TkMul) opcode = OpMul;
        else if (optk == TkDiv) opcode = OpDiv;
        else if (optk == TkRem) opcode = OpRem;
        else break;
        ++tkIter;
        add_ins(OP(OpPush, RegEax, 0, 0), 0);
        expr_post();
        add_ins(OP(OpPop, RegEbx, 0, 0), 0);
        add_ins(OP(opcode, RegEax, RegEbx, RegEax), 0);
    }
}

void expr_add() {
    expr_mul();
    for (;;) {
        int optk = tks[tkIter].kind; int opcode;
        if (optk == TkAdd) opcode = OpAdd;
        else if (optk == TkSub) opcode = OpSub;
        else break;
        ++tkIter;
        add_ins(OP(OpPush, RegEax, 0, 0), 0);
        expr_mul();
        add_ins(OP(OpPop, RegEbx, 0, 0), 0);
        add_ins(OP(opcode, RegEax, RegEbx, RegEax), 0);
    }
}

void expr() {
    expr_add();
}

void stmt() {
    int kind = tks[tkIter].kind;
    if (kind == KwRet) {
        if (tks[++tkIter].kind != TkSC) expr();
        add_ins(OpRet, 0);
        expect(TkSC);
    } else if (kind == TkLB) {
        enter_scope();
        ++tkIter;
        while (tks[tkIter].kind != TkRB) {
            kind = tks[tkIter].kind;
            if (kind == KwInt) {
                int offset;
                expect_type(&kind, &offset);
                add_ins(OpSub | (RegEsp << 8) | (RegEsp << 16) | (Imme << 24), offset);
                // TODO: add to symbol table
                expect(TkId);
                expect(TkSC);
            } else {
                stmt();
            }
        }
        ++tkIter;

        exit_scope();
    } else {
        expr();
        expect(TkSC);
    }
}

// HACK only parse return stmt for now
void obj() {
    // type
    int kind; int offset;
    expect_type(&kind, &offset);

    // int id = expect(TkId);
    expect(TkId);

    expect(TkLP);
    expect(TkRP);

    stmt();
}

void gen() {
    enter_scope();

    // TODO: enum, struct
    obj();

    exit_scope();
}

void exec() {
    int pc = 0;
    while (pc < insNum) {
        int op = ins[pc].op;
        int imme = ins[pc].imme;
        int dest = (op & 0xFF00) >> 8;
        int src1 = (op & 0xFF0000) >> 16;
        int src2 = (op & 0xFF000000) >> 24;
        int value = src2 == Imme ? imme : regs[src2];
        op = op & 0xFF;
        if (op == OpMov) {
            regs[dest] = value;
        } else if (op == OpPush) {
            regs[RegEsp] -= 4;
            ((int*)stack)[regs[RegEsp] >> 2] = regs[dest];
        } else if (op == OpPop) {
            regs[dest] = ((int*)stack)[regs[RegEsp] >> 2];
            regs[RegEsp] += 4;
        } else if (op == OpAdd) {
            regs[dest] = regs[src1] + value;
        } else if (op == OpSub) {
            regs[dest] = regs[src1] - value;
        } else if (op == OpMul) {
            regs[dest] = regs[src1] * value;
        } else if (op == OpDiv) {
            regs[dest] = regs[src1] / value;
        } else if (op == OpRem) {
            regs[dest] = regs[src1] % value;
        } else if (op == OpRet) {
            // TODO: implement
            break;
        } else if (op == CPrintf) {
            int slot = regs[RegEsp] >> 2;
            printf((char*)((int*)stack)[slot + 7],
                   ((int*)stack)[slot + 6],
                   ((int*)stack)[slot + 5],
                   ((int*)stack)[slot + 4],
                   ((int*)stack)[slot + 3],
                   ((int*)stack)[slot + 2],
                   ((int*)stack)[slot + 1],
                   ((int*)stack)[slot + 0]);
        } else {
            panic("Invalid op code");
        }
        ++pc;
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s [options] file\n", argv[0]);
        printf("Options:\n");
        exit(1);
    }

    // initialization
    stack = calloc(1, STACK_SIZE);
    data = calloc(1, DATA_SECTION_SIZE);
    regs[RegEsp] = STACK_SIZE;

    // store source to buffer
    void* fp = fopen(argv[1], "r");
    for (p = src; (p[0] = getc(fp)) != EOF; ++p) {
        if ((p - src) >= MAX_SRC) {
            panic("src buffer overflow");
        }
    }
    p[0] = '\0';
    fclose(fp);
    p = src;

    // set global values
    ln = 1;

    DEVPRINT("-------- lex --------\n");
    lex();
    // debug
    dump_tks();

    DEVPRINT("-------- code --------\n");
    gen();

    // debug
    dump_code();

    DEVPRINT("-------- exec --------\n");
    exec();

    free(stack);
    free(data);

    DEVPRINT("-------- exiting --------\n");
    DEVPRINT("script '%s' exit with code %d\n", argv[1], regs[RegEax]);
    return regs[RegEax];
}
