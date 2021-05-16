#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// definitions
enum { TkErr = 0,
       TkLP = '(', TkRP = ')',
       TkLB = '{', TkRB = '}',
       TkLS = '[', TkRS = ']',
       TkSC = ';',
       TkAdd = '+', TkSub = '-', TkMul = '*', TkDiv = '/', TkRem = '%',
       TkInt = 128, TkId, TkStr, TkChar,
       KwOffset, KwInt, KwRet };

enum { TVoid, TInt, TChar, TPtr };

enum { Global, Local };

// vm
enum { OpRet = 1, OpMov, OpPush, OpPop, OpAdd, OpSub, OpMul, OpDiv, OpRem };
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
    int type;
    int offset; // offset to ebp
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

char src[MAX_SRC]; char *p; int ln;

// tokens
struct Token tks[MAX_TOKEN]; int tkNum; int tkIter;

// symbol table
struct Symbol syms[MAX_SYMBOL];
int symCnt;

int regs[Imme];
struct Ins ins[MAX_INS];
int insNum;

int* stack;
char* data;

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
            char* kw = "int return "; pp = kw;
            for (int offset = 1; (pp = strchr(kw, ' ')); kw = pp + 1, ++offset) {
                if (strncmp(kw, tks[tkNum].start, pp - kw) == 0) {
                    tks[tkNum].kind = KwOffset + offset;
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
        } else if ((pp = strchr("()[]{};+-*/%", *p))) {
            tks[tkNum].kind = *pp; tks[tkNum].ln = ln; tks[tkNum].start = p;
            tks[tkNum++].end = ++p;
        } else {
            printf("error: strayed char %c on line %d\n", *p, ln);
            exit(1);
        }
    }
}

int expect(int kind) {
    if (tks[tkIter].kind != kind) {
        printf("expect %s on line %d, got %.*s\n", tk2str(kind), tks[tkIter].ln, tks[tkIter].end - tks[tkIter].start, tks[tkIter].start);
        exit(1);
    }
    return tkIter++;
}

void expect_type(int* kind, int* byte) {
    int type = tks[tkIter].kind;
    if (type == KwInt) {
        *kind = TInt;
        *byte = 4;
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

void expr_atomic() {
    int kind = tks[tkIter].kind;
    if (kind == TkInt) {
        int val = str2int(tks[tkIter].start, tks[tkIter].end);
        add_ins(OpMov | (RegEax << 8) | (Imme << 16), val);
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

void expr_mul() {
    expr_atomic();
    for (;;) {
        int optk = tks[tkIter].kind; int opcode;
        if (optk == TkMul) opcode = OpMul;
        else if (optk == TkDiv) opcode = OpDiv;
        else if (optk == TkRem) opcode = OpRem;
        else break;
        ++tkIter;
        add_ins(OpPush | 0x0100, 0);
        expr_atomic();
        add_ins(OpPop | 0x0200, 0);
        add_ins(opcode | 0x01020100, 0);
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
        add_ins(OpPush | 0x0100, 0);
        expr_mul();
        add_ins(OpPop | 0x0200, 0);
        add_ins(opcode | 0x01020100, 0);
    }
}

void expr() {
    expr_add();
}

void stmt() {
    int kind = tks[tkIter++].kind;
    if (kind == KwRet) {
        if (tks[tkIter].kind != TkSC) expr();
        add_ins(OpRet, 0);
        expect(TkSC);
    } else {
        panic("TODO: implement stmt()");
    }
}

// HACK only parse return stmt for now
void obj() {
    // type
    int kind; int byte;
    expect_type(&kind, &byte);

    // int id = expect(TkId);
    expect(TkId);

    expect(TkLP);
    expect(TkRP);
    expect(TkLB);

    stmt();

    expect(TkRB);
}

void gen() {
    // enum
    // struct
    // object
    obj();
}

void exec() {
    int pc = 0;
    while (pc < insNum) {
        int op = ins[pc].op;
        int imme = ins[pc].imme;
        int dest = (op & 0xFF00) >> 8;
        int src1 = (op & 0xFF0000) >> 16;
        int src2 = (op & 0xFF000000) >> 24;
        op = op & 0xFF;
        if (op == OpMov) {
            int value = src1 == Imme ? imme : regs[src1];
            regs[dest] = value;
        } else if (op == OpRet) {
            // TODO: implement
            break;
        } else if (op == OpAdd) {
            regs[dest] = regs[src1] + regs[src2];
        } else if (op == OpSub) {
            regs[dest] = regs[src1] - regs[src2];
        } else if (op == OpMul) {
            regs[dest] = regs[src1] * regs[src2];
        } else if (op == OpDiv) {
            regs[dest] = regs[src1] / regs[src2];
        } else if (op == OpRem) {
            regs[dest] = regs[src1] % regs[src2];
        } else if (op == OpPush) {
            --regs[RegEsp];
            stack[regs[RegEsp]] = regs[dest];
        } else if (op == OpPop) {
            regs[dest] = stack[regs[RegEsp]];
            ++regs[RegEsp];
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
    stack = calloc(1, 4 * STACK_SIZE);
    data = calloc(1, DATA_SECTION_SIZE);
    regs[RegEsp] = STACK_SIZE;

    // store source to buffer
    FILE* fp = fopen(argv[1], "r");
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

    printf("-------- lex --------\n");
    lex();
    // debug
    dump_tks();

    printf("-------- gen --------\n");
    gen();

    // debug
    dump_code();

    printf("-------- exe --------\n");
    exec();

    free(stack);

    printf("script '%s' exit with code %d\n", argv[1], regs[RegEax]);
    return regs[RegEax];
}
