#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// TODO: replace
//#define for @
#define do @
//- no for
//- continue
//- ?:

#if !defined(_TEST)
#define DEVPRINT(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEVPRINT(...)
#endif

#define ERROR(...) { printf("\n%s:", prog); printf(__VA_ARGS__); exit(1); }

// definitions
enum { TkErr = 0,
       TkLP = '(', TkRP = ')',
       TkLB = '{', TkRB = '}',
       TkLS = '[', TkRS = ']',
       TkSC = ';', TkComma = ',',
       TkAdd = '+', TkSub = '-', TkMul = '*', TkDiv = '/', TkRem = '%',
       TkEq = '=',
       TkInt = 128, TkId, TkStr, TkChar,
       _KwOffset,
       KwInt, KwIf, KwElse, KwRet, KwPrintf };

enum { TVoid, TInt, TChar, TPtr };

enum { Undefined, Global, Param, Local, Func };

// vm
enum { _Offset = 1,
       OpAdd,   // dst = x + y
       OpSub,   // dst = x - y
       OpMul,   // dst = x * y
       OpDiv,   // dst = x / y
       OpRem,   // dst = x % y
       OpPush,  // esp -= 4; [esp] = x
       OpPop,   // x = [esp]; esp += 4
       OpMov,   // mov x, y or mov x, 2
       OpStore, // store [x] y
       OpLoad,  // load x [y]
       OpLoadc, // load x 0xFF & [y]
       OpRet,   // ret
       OpJZ,  // if eax == 0 jump
       OpJump,
       CPrintf  // printf
       };
enum { RegEax = 1, RegEbx, RegEcx, RegEdx, RegEsp, RegEbp, Imme };

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

// |  op  |  dst  |  src1  |  src2  |
// | 0-7  |  8-15 | 16-24  | 25-31  |
struct Ins {
    int op;
    int imme;
};

// globals
// source code
#define MAX_TOKEN (1 << 10)
#define MAX_SYMBOL (1 << 8)
#define MAX_INS (1 << 12)
#define STACK_SIZE (1 << 16)
#define DATA_SECTION_SIZE (1 << 12)
#define INT_SIZE 4
#define MAX_PRINF_ARGS 8

// source file
char* src; char* p; int srclen; int ln;
const char* prog;

// tokens
struct Token tks[MAX_TOKEN]; int tkNum; int tkIter;

int regs[Imme];
struct Ins ins[MAX_INS];
int insNum;

char* stack;
char* data;
int ds;

#define MAX_SCOPE 128
int scopeId;
int scope[MAX_SCOPE];
int scopeCnt;

struct Symbol syms[MAX_SYMBOL];
int symCnt;

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

// debug only
#include "debug.inl"

void lex() {
    p = src;
    ln = 1;
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
            char* kw = "int if else return printf "; pp = kw;
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
            ERROR("%d: strayed char '%c'\n", ln, *p);
        }
    }
}

void enter_scope() {
    if (scopeCnt >= MAX_SCOPE) {
        panic("scope overflow");
    }

    scope[scopeCnt++] = ++scopeId;
}

// debug
void debugprintsymbols() {
    for (int i = 0; i < symCnt; ++i) {
        int idx = syms[i].tkIdx;
        char* start = tks[idx].start;
        char* end = tks[idx].end;
        int len = end - start;

        DEVPRINT("%.*s: %d, scope %d\n", len, start, syms[i].address, syms[i].scope);
    }
}

void exit_scope() {
    if (scopeCnt <= 0) {
        panic("scope overflow");
    }

    // debugprintsymbols();
    for (int i = symCnt - 1; syms[i].scope == scope[scopeCnt - 1]; --i) {
        int idx = syms[i].tkIdx;
        char* start = tks[i].start;
        char* end = tks[i].end;
        int len = end - start;
        DEVPRINT("popping %.*s\n", len, start);
        --symCnt;
    }

    --scopeCnt;
}

int expect(int kind) {
    if (tks[tkIter].kind != kind) {
        ERROR("%d: expected %s, got '%.*s'\n",
            tks[tkIter].ln,
            tk2str(kind),
            tks[tkIter].end - tks[tkIter].start,
            tks[tkIter].start);
    }
    return tkIter++;
}

int expect_type() {
    int type = tks[tkIter].kind;
    if (type == KwInt) {
        ++tkIter;
        return TInt;
    } else {
        ERROR("%d: expected type specifier, got '%.*s'\n",
            tks[tkIter].ln,
            tks[tkIter].end - tks[tkIter].start,
            tks[tkIter].start);
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

#define OP2(op, dest) ((op) | (dest << 8))
#define OP3(op, dest, src1) ((op) | (dest << 8) | (src1 << 16))
#define OP(op, dest, src1, src2) ((op) | (dest << 8) | (src1 << 16) | (src2 << 24))

void expr_post() {
    char* start = tks[tkIter].start;
    char* end = tks[tkIter].end;
    int ln = tks[tkIter].ln;
    int kind = tks[tkIter++].kind;
    int len = end - start;
    if (kind == TkInt) {
        int val = str2int(start, end);
        add_ins(OP(OpMov, RegEax, 0, Imme), val);
    } else if (kind == TkStr) {
        if (ds > DATA_SECTION_SIZE / 2)
            panic("data is running low");

        add_ins(OP(OpMov, RegEax, 0, Imme), (int)(data + ds));
        for (int i = 1; i < len - 1; ++i) {
            int c = start[i];
            if (c == '\\') {
                c = start[++i];
                if (c == 'n') c = '\n';
                else panic("handle escape sequence");
            }
            data[ds++] = c;
        }
        data[ds++] = 0;
        ds = (ds + 3) & (0xFFFFFFFF << 2); // align data
    } else if (kind == TkLP) {
        expr();
        expect(TkRP);
    } else if (kind == TkId) {
        int address = 0;
        int type = 0;
        for (int i = symCnt - 1; i >= 0; --i) {
            int tmp = syms[i].tkIdx;
            if (strncmp(tks[tmp].start, start, len) == 0) {
                address = syms[i].address;
                type = syms[i].type;
                break;
            }
        }

        if (type == Undefined) {
            ERROR("%d: '%.*s' undeclared\n", ln, len, start);
        }

        if (type == Func) {
            panic("TODO: implement func call");
        } else if (type == Global) {
            panic("TODO: implement globlal variable");
        } else {
            add_ins(OP(OpSub, RegEdx, RegEbp, Imme), address);
            add_ins(OP(OpLoad, RegEax, RegEdx, 0), 0);
        }
    } else if (kind == KwPrintf) {
        expect(TkLP);
        int argc = 0;
        for (; tks[tkIter].kind != TkRP; ++argc) {
            if (argc > 0) expect(TkComma);
            expr();
            add_ins(OP2(OpPush, RegEax), 0);
        }
        if (argc > MAX_PRINF_ARGS) panic("printf supports at most %d args");
        add_ins(OP(OpSub, RegEsp, RegEsp, Imme), (MAX_PRINF_ARGS - argc) << 2);
        add_ins(CPrintf, argc);
        add_ins(OP(OpAdd, RegEsp, RegEsp, Imme), MAX_PRINF_ARGS << 2);
        expect(TkRP);
    } else {
        ERROR("%d: expected expression, got '%.*s'\n", ln, len, start);
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
        add_ins(OP2(OpPush, RegEax), 0);
        expr_post();
        add_ins(OP2(OpPop, RegEbx), 0);
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
        add_ins(OP2(OpPush, RegEax), 0);
        expr_mul();
        add_ins(OP2(OpPop, RegEbx), 0);
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
    } else if (kind == KwIf) {
        // if eax == 0, jmp L1
        // ...
        // jump L2:
        // L1: ...
        // L2: ...
        ++tkIter;
        expect(TkLP);
        expr();
        expect(TkRP);
        int jump1Loc = insNum;
        add_ins(OpJZ, 0);
        stmt();
        int jump2Loc = insNum;
        add_ins(OpJump, insNum + 1);
        ins[jump1Loc].imme = insNum;
        if (tks[tkIter].kind == KwElse) {
            ++tkIter; // skip else
            stmt();
            ins[jump2Loc].imme = insNum;
        }
    } else if (kind == TkLB) {
        enter_scope();
        ++tkIter;
        int restore = 0;
        while (tks[tkIter].kind != TkRB) {
            kind = tks[tkIter].kind;
            if (kind == KwInt /*TODO: Char, Ptr*/) {
                expect_type();
                add_ins(OP(OpSub, RegEsp, RegEsp, Imme), 4);
                char* start = tks[tkIter].start;
                int len = tks[tkIter].end - start;
                for (int i = symCnt - 1; syms[i].scope == scope[scopeCnt - 1]; --i) {
                    int tmpId = syms[i].tkIdx;
                    if (strncmp(tks[tmpId].start, start, len) == 0) {
                        ERROR(
                            "%d: redeclaration of '%.*s', previously defined on line %d\n",
                            tks[tkIter].ln, len, start, tks[tmpId].ln);
                    }
                }

                if (symCnt >= MAX_SYMBOL) {
                    panic("symbol overflow");
                }

                int prev = symCnt - 1;
                syms[symCnt].address = 4;
                if (prev >= 0 && syms[prev].type == Local) {
                    syms[symCnt].address += syms[prev].address;
                }

                syms[symCnt].type = Local;
                syms[symCnt].tkIdx = tkIter;
                syms[symCnt].scope = scope[scopeCnt - 1];
                ++symCnt;
                ++tkIter;

                if (tks[tkIter].kind == TkEq) {
                    ++tkIter;
                    expr();
                    add_ins(OP(OpStore, RegEsp, RegEax, 0), 0);
                }

                ++restore;
                expect(TkSC);
            } else {
                stmt();
            }
        }
        ++tkIter;

        if (restore) add_ins(OP(OpAdd, RegEsp, RegEsp, Imme), restore << 2);
        exit_scope();
    } else if (kind == TkSC) {
        ++tkIter;
    } else {
        expr();
        expect(TkSC);
    }
}

void obj() {
    // type
    expect_type();

    // int id = expect(TkId);
    expect(TkId);

    expect(TkLP);
    expect(TkRP);

    // save frame
    add_ins(OP2(OpPush, RegEbp), 0);
    add_ins(OP(OpMov, RegEbp, 0, RegEsp), 0);
    stmt();
    // clear frame
}

void gen() {
    enter_scope();

    obj();

    exit_scope();
}

void printvm() {
    DEVPRINT("********** VM begin **********\n");
    DEVPRINT("eax: %d\n", regs[RegEax]);
    DEVPRINT("ebx: %d\n", regs[RegEbx]);
    DEVPRINT("ecx: %d\n", regs[RegEcx]);
    DEVPRINT("edx: 0x%X\n", regs[RegEdx]);
    DEVPRINT("esp: 0x%X\n", regs[RegEsp]);
    DEVPRINT("ebp: 0x%X\n", regs[RegEbp]);
    DEVPRINT("********** VM end **********\n");
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
        } else if (op == OpStore) {
            ((int*)stack)[regs[dest] >> 2] = regs[src1];
        } else if (op == OpLoad) {
            regs[dest] = ((int*)stack)[regs[src1] >> 2];
        } else if (op == OpJump) {
            pc = imme - 1; // because of the ++pc at the end
        } else if (op == OpJZ) {
            if (!regs[RegEax]) {
                pc = imme - 1; // because of the ++pc at the end
            }
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
        pc = pc + 1;
    }
}

int main(int argc, char **argv) {
    prog = argv[0];
    p = strrchr(prog, '\\');
    prog = p ? p + 1 : prog;
    p = strrchr(prog, '/');
    prog = p ? p + 1 : prog;

    if (argc != 2) {
        printf("Usage: %s [options] file\n", prog);
        printf("Options:\n");
        exit(1);
    }

    // initialization
    stack = calloc(1, STACK_SIZE);
    data = calloc(1, DATA_SECTION_SIZE);
    regs[RegEsp] = STACK_SIZE;

    // store source to buffer
    void* fp = fopen(argv[1], "r");
    if (!fp) {
        printf("file '%s' does not exist\n", argv[1]);
        return 1;
    }

    fseek(fp, 0, 2); // SEEK_END
    srclen = ftell(fp);
    fseek(fp, 0, 0); // SEEK_SET
    src = calloc(1, srclen + 1); // pad
    fread(src, 1, srclen, fp);
    fclose(fp);

    lex();
    // DEVPRINT("-------- lex --------\n");
    // dump_tks();

    gen();
    DEVPRINT("-------- code --------\n");
    dump_code();

    DEVPRINT("-------- exec --------\n");
    exec();

    free(stack);
    free(data);
    free(src);

    DEVPRINT("-------- exiting --------\n");
    DEVPRINT("script '%s' exit with code %d\n", argv[1], regs[RegEax]);
    return regs[RegEax];
}
