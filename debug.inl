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
        case TkComma: return "','";
        case TkSC: return "';'";
        case TkEq: return "'='";
        case TkAdd: return "add";
        case TkSub: return "sub";
        case TkMul: return "mul";
        case TkDiv: return "div";
        case TkRem: return "rem";
        case KwInt: return "Int";
        case KwRet: return "Ret";
        case KwPrintf: return "Print";
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

void dump_tk(int i) {
    DEVPRINT("[ %-5s] ln:%d: [%.*s]\n", tk2str(tks[i].kind), tks[i].ln, tks[i].end - tks[i].start, tks[i].start);
}

void dump_tks() {
    for (int i = 0; i < tkNum; ++i) {
        dump_tk(i);
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
        DEVPRINT("[ 0x%04X ]", (pc + 1) << 2);
        switch (op) {
        case OpMov:
            DEVPRINT("  mov %s, ", reg2str(dest));
            if (src2 == Imme) DEVPRINT("%d\n", imme);
            else DEVPRINT("%s\n", reg2str(src2));
            break;
        case OpRet: 
            DEVPRINT("  ret\n");
            break;
        case OpAdd: 
        case OpSub: 
        case OpMul: 
        case OpDiv: 
        case OpRem: 
            DEVPRINT("  %s %s, %s, ", op2str(op), reg2str(dest), reg2str(src1));
            if (src2 == Imme) DEVPRINT("%d\n", imme);
            else DEVPRINT("%s\n", reg2str(src2));
            break;
        case OpPush:
        case OpPop:
            DEVPRINT("  %s %s\n", op2str(op), reg2str(dest));
            break;
        case CPrintf:
            DEVPRINT("  DEVPRINT\n");
            break;
        default:
            panic("Invalid op code");
            break;
        }
    }
    DEVPRINT("%d lines of code\n", insNum);
}
// debug only
