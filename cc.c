#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// dev only
void panic(char* fmt) {
    printf("[panic] %s\n", fmt);
    exit(1);
}
// dev only

char src[1 << 12];
char *p;
int ln;

enum { TkErr = 0, TkInt = 128, TkId, TkStr, TkChar,
       KwOffset, KwInt, KwRet
       };

enum { TkLP = '(', TkRP = ')',
       TkLB = '{', TkRB = '}',
       TkLS = '[', TkRS = ']',
       TkSC = ';' };

struct Token {
    int kind;
    int ln;
    char* start;
    char* end;
};

struct Token tks[1 << 10];
int tkNum;

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

void usage(char *prog) {
    printf("Usage: %s [options] file\n", prog);
    printf("Options:\n");
}

char* tk2str(int kd) {
    switch (kd) {
        case TkErr: return "ERR";
        case TkInt: return "INT";
        case TkId: return "ID";
        case TkStr: return "STR";
        case TkChar: return "CHAR";
        case TkLP: return "LP";
        case TkRP: return "RP";
        case TkLB: return "LB";
        case TkRB: return "RB";
        case TkLS: return "LS";
        case TkRS: return "RS";
        case TkSC: return "SC";
        case KwInt: return "Int";
        case KwRet: return "Ret";
        default: panic("unknown token"); return "<error>";
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        usage(argv[0]);
        exit(1);
    }

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

    // lex
    lex();

    for (int i = 0; i < tkNum; ++i) {
        struct Token* tk = &tks[i];
        printf("[ %-4s] ln:%d: %.*s\n", tk2str(tk->kind), tk->ln, tk->end - tk->start, tk->start);
    }

    printf("exit with code 0x0\n");
    return 0;
}
