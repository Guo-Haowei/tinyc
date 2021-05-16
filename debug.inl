// debug only

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

char* op2str(int op) {
    switch (op) {
        case OpRet: return "ret";
        case OpMov: return "mov";
        default: panic("unknown token"); return "<error>";
    }
}

char* reg2str(int reg) {
    switch (reg) {
        case RegEax: return "eax";
        case RegEbx: return "ebx";
        default: panic("unknown reg"); return "<error>";
    }
}

void dump_tks() {
    for (int i = 0; i < tkNum; ++i) {
        struct Token* tk = &tks[i];
        printf("[ %-4s] ln:%d: %.*s\n", tk2str(tk->kind), tk->ln, tk->end - tk->start, tk->start);
    }
}

void dump_code() {
    for (int pc = 0; pc < insNum; ++pc) {
        int op = ins[pc].op;
        int imme = ins[pc].imme;
        int dest = (op & 0xFF00) >> 8;
        int src1 = (op & 0xFF0000) >> 16;
        int src2 = (op & 0xFF000000) >> 24;
        op = op & 0xFF;
        if (op == OpMov) {
            printf("    mov %s, ", reg2str(dest));
            if (src1 == Imme) printf("%d\n", imme);
            else printf("%s\n", reg2str(src1));
        } else if (op == OpRet) {
            printf("    ret\n");
        } else {
            panic("Invalid op code");
        }
    }
}
// debug only
