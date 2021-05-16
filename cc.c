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
       TkInt = 128, TkId, TkStr, TkChar,
       KwOffset, KwInt, KwRet };

// globals
char src[1 << 12];
char *p;
int ln;

struct Token {
    int kind;
    int ln;
    char* start;
    char* end;
};

struct Token tks[1 << 10];
int tkNum;
int tkIter;

// vm
enum { OpRet = 1, OpMov };
enum { RegEax, RegEbx, RegEcx, RegEdx, RegEsp, RegEbp, Imme };
int regs[Imme];

// mov eax, ebx
// add eax, ebx
// |------|-- src --|-- dst --|-- op --|
struct Ins {
    int op;
    int imme;
};

struct Ins ins[1 << 12];
int insNum;
char* stack;

// debug only
#include "debug.inl"
// debug only

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

void lex() {
    char* pp;
    while (*p) {
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
        } else if ((pp = strchr("()[]{};", *p))) {
            tks[tkNum].kind = *pp; tks[tkNum].ln = ln; tks[tkNum].start = p;
            tks[tkNum++].end = ++p;
        } else {
            printf("error: strayed char %c on line %d\n", *p, ln);
            exit(1);
        }

        if (tkNum >= (sizeof(tks) / sizeof(tks[0]))) {
            panic("tks overflow\n");
        }
    }
}

int expect(int kind) {
    if (tks[tkIter].kind != kind) {
        printf("expect %s on line %d, got %.*s\n", tk2str(kind), tks[tkIter].ln, tks[tkIter].end - tks[tkIter].start, tks[tkIter].start);
        exit(-1);
    }
    return tkIter++;
}

// HACK only parse return stmt for now
void gen() {
    expect(KwInt);
    int id = expect(TkId);
    expect(TkLP);
    expect(TkRP);
    expect(TkLB);

    expect(KwRet);
    int num = expect(TkInt);
    int val = str2int(tks[num].start, tks[num].end);

    ins[insNum].op = OpMov | (RegEax << 8) | (Imme << 16);
    ins[insNum].imme = val;
    ins[insNum + 1].op = OpRet;
    insNum += 2;

    expect(TkSC);
    expect(TkRB);
}

void exec() {
    int pc = 0;
    while (pc < insNum) {
        int op = ins[pc].op;
        int imme = ins[pc].imme;
        int dest = (op & 0xFF00) >> 8;
        int src = (op & 0xFF0000) >> 16;
        op = op & 0xFF;
        // printf("op %d dest: %d src: %d\n", op, dest, src);
        if (op == OpMov) {
            int value = src == Imme ? imme : regs[src];
            regs[dest] = value;
            ++pc;
        } else if (op == OpRet) {
            // TODO: implement
            ++pc;
        } else {
            panic("Invalid op code");
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s [options] file\n", argv[0]);
        printf("Options:\n");
        exit(1);
    }

    // initialization
    const int stackSize = 1 << 16;
    stack = calloc(1, stackSize);
    regs[RegEsp] = stackSize - 1;

    // store source to buffer
    FILE* fp = fopen(argv[1], "r");
    for (p = src; (p[0] = getc(fp)) != EOF; ++p) {
        if ((p - src) >= sizeof(src)) {
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
    return 0;
}
