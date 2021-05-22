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

char *ram, *g_src;
int memory;
int *g_tks, g_tkCnt, g_tkIter;

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
            g_tks[(g_tkCnt) * TokenSize + Ln] = ln;
            g_tks[(g_tkCnt) * TokenSize + Start] = p;
            if (((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')) || *p == '_') {
                g_tks[(g_tkCnt) * TokenSize + Kind] = Id;
                ++p; while (((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')) || (*p >= '0' && *p <= '9') || *p == '_') { ++p; }
                g_tks[(g_tkCnt) * TokenSize + End] = p;
                char *keywords = "int\0     char\0    void\0    break\0   continue\0"
                                 "else\0    enum\0    if\0      return\0  while\0   "
                                 "printf\0  fopen\0   fgetc\0   malloc\0  memset\0  "
                                 "exit\0    ";
                int i = 0, start = g_tks[(g_tkCnt) * TokenSize + Start], len = g_tks[(g_tkCnt) * TokenSize + End] - start;
                while (i < range) {
                    char* kw = keywords + (i * 9);
                    int kwlen = strlen(kw);
                    if (kwlen == len && streq(start, kw, 8)) {
                        g_tks[(g_tkCnt) * TokenSize + Kind] = _KeywordStart + i + 1;
                        break;
                    }
                    ++i;
                }
                g_tkCnt += 1;
            } else if (*p == '0' && p[1] == 'x') {
                g_tks[(g_tkCnt) * TokenSize + Kind] = CInt;
                int result = 0;
                p += 2; while(((*p >= '0' && *p <= '9') || (*p >= 'A' && *p <= 'F'))) {
                    result = (result << 4) + ((*p < 'A') ? (*p - '0') : (*p - 55));
                    ++p;
                }
                g_tks[(g_tkCnt) * TokenSize + Value] = result;
                g_tks[(g_tkCnt++) * TokenSize + End] = p;
            } else if ((*p >= '0' && *p <= '9')) {
                g_tks[(g_tkCnt) * TokenSize + Kind] = CInt;
                int result = 0;
                while ((*p >= '0' && *p <= '9')) { result = result * 10 + (*p - '0'); ++p; }
                g_tks[(g_tkCnt) * TokenSize + Value] = result;
                g_tks[(g_tkCnt++) * TokenSize + End] = p;
            } else if (*p == '"') {
                g_tks[(g_tkCnt) * TokenSize + Kind] = CStr;
                ++p; while (*p != '"') { ++p; };
                g_tks[(g_tkCnt++) * TokenSize + End] = ++p;
            } else if (*p == 39) {
                g_tks[(g_tkCnt) * TokenSize + Kind] = CChar;
                g_tks[(g_tkCnt) * TokenSize + Value] = p[1];
                g_tks[(g_tkCnt++) * TokenSize + End] = (p += 3);
            } else {
                g_tks[(g_tkCnt) * TokenSize + Kind] = *p;
                if ((*p == '=' && p[1] == '=')) { g_tks[(g_tkCnt) * TokenSize + Kind] = TkEq; ++p; }
                else if ((*p == '!' && p[1] == '=')) { g_tks[(g_tkCnt) * TokenSize + Kind] = TkNeq; ++p; }
                else if ((*p == '&' && p[1] == '&')) { g_tks[(g_tkCnt) * TokenSize + Kind] = TkAnd; ++p; }
                else if ((*p == '|' && p[1] == '|')) { g_tks[(g_tkCnt) * TokenSize + Kind] = TkOr; ++p; }
                else if (*p == '+') {
                    if (p[1] == '+') { g_tks[(g_tkCnt) * TokenSize + Kind] = TkInc; ++p; }
                    else if (p[1] == '=') { g_tks[(g_tkCnt) * TokenSize + Kind] = TkAddTo; ++p; }
                } else if (*p == '-') {
                    if (p[1] == '-') { g_tks[(g_tkCnt) * TokenSize + Kind] = TkDec; ++p; }
                    else if (p[1] == '=') { g_tks[(g_tkCnt) * TokenSize + Kind] = TkSubFrom; ++p; }
                } else if (*p == '>') {
                    if (p[1] == '=') { g_tks[(g_tkCnt) * TokenSize + Kind] = TkGe; ++p; }
                    else if (p[1] == '>') { g_tks[(g_tkCnt) * TokenSize + Kind] = RShift; ++p; }
                } else if (*p == '<') {
                    if (p[1] == '=') { g_tks[(g_tkCnt) * TokenSize + Kind] = TkLe; ++p; }
                    else if (p[1] == '<') { g_tks[(g_tkCnt) * TokenSize + Kind] = LShift; ++p; }
                }
                g_tks[(g_tkCnt++) * TokenSize + End] = ++p;
            }
        }
    }
    return;
}

void dump_tokens() {
    int indent = 0, i = 0, ln = 0;
    while (i < g_tkCnt) {
        int tkln = g_tks[(i) * TokenSize + Ln];
        int kind = g_tks[(i) * TokenSize + Kind];
        int start = g_tks[(i) * TokenSize + Start];
        int end = g_tks[(i) * TokenSize + End];
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
    memory = 2 * (1 << 20) * argc;
    ram = malloc(memory);
    int src_reserved = 1 << 18; {
        g_src = ram + (memory - src_reserved);
        int c, i = 0;;
        while ((c = fgetc(fp)) != -1) { g_src[i++] = c; }
        g_src[i] = 0;
    }
    int tk_reserved = 4 * TokenSize * (src_reserved >> 1); {
        g_tks = ram + (memory - src_reserved - tk_reserved);
        lex();
        dump_tokens();
    }
    return 0;
}
