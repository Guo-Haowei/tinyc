// debug only

char* tk2str(int kd) {
    switch (kd) {
        case TkErr: return "ERR";
        case TkInt: return "INT";
        case TkId: return "ID";
        case TkStr: return "STR";
        case TkChar: return "CHAR";
        case TkLP: return "'('";
        case TkRP: return "')'";
        case TkLB: return "'{'";
        case TkRB: return "'}'";
        case TkLS: return "'['";
        case TkRS: return "']'";
        case TkSC: return "';'";
        case TkAdd: return "add";
        case TkSub: return "sub";
        case TkMul: return "mul";
        case TkDiv: return "div";
        case TkRem: return "rem";
        case KwInt: return "Int";
        case KwRet: return "Ret";
        default: panic("unknown token"); return "<error>";
    }
}

char* op2str(int op) {
    switch (op) {
        case OpRet: return "ret";
        case OpMov: return "mov";
        case OpAdd: return "add";
        case OpSub: return "sub";
        case OpMul: return "mul";
        case OpDiv: return "div";
        case OpRem: return "rem";
        case OpPush: return "push";
        case OpPop: return "pop";
        default: panic("unknown token"); return "<error>";
    }
}

char* reg2str(int reg) {
    switch (reg) {
        case RegEax: return "eax";
        case RegEbx: return "ebx";
        case RegEbp: return "ebp";
        case RegEsp: return "esp";
        default: panic("unknown reg"); return "<error>";
    }
}

void dump_tks() {
    for (int i = 0; i < tkNum; ++i) {
        struct Token* tk = &tks[i];
        printf("[ %-4s] ln:%d: [%.*s]\n", tk2str(tk->kind), tk->ln, tk->end - tk->start, tk->start);
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
        switch (op) {
        case OpMov:
            printf("    mov %s, ", reg2str(dest));
            if (src1 == Imme) printf("%d\n", imme);
            else printf("%s\n", reg2str(src1));
            break;
        case OpRet: 
            printf("    ret\n");
            break;
        case OpAdd: 
        case OpSub: 
        case OpMul: 
        case OpDiv: 
        case OpRem: 
            printf("    %s %s, %s, %s\n", op2str(op), reg2str(dest), reg2str(src1), reg2str(src2));
            break;
        case OpPush:
        case OpPop:
            printf("    %s %s\n", op2str(op), reg2str(dest));
            break;
        default:
            panic("Invalid op code");
            break;
        }
    }
}
// debug only
