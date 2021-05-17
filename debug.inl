// debug only

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
        case OpEq: return "EQ";
        case OpNe: return "NE";
        case OpLt: return "LT";
        case OpLe: return "LE";
        case OpGt: return "GT";
        case OpGe: return "GE";
        default: panic("unknown token"); return "<error>";
    }
}

char* reg2str(int reg) {
    switch (reg) {
        case RegEax: return "eax";
        case RegEbx: return "ebx";
        case RegEcx: return "ecx";
        case RegEdx: return "edx";
        case RegEsp: return "esp";
        case RegEbp: return "ebp";
        default: printf("%d\n", reg);panic("unknown reg"); return "<error>";
    }
}

void dump_tk(int i) {
    DEVPRINT("[ %-7s] ln:%d: [%.*s]\n", tk2str(tks[i].kind), tks[i].ln, tks[i].end - tks[i].start, tks[i].start);
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
        DEVPRINT("[ %4d ]", pc);
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
        case OpEq:
        case OpNe:
        case OpGt:
        case OpGe:
        case OpLt:
        case OpLe:
            DEVPRINT("  %s %s, %s, %s\n", op2str(op), reg2str(dest), reg2str(src1), reg2str(src2));
            break;
        case OpPush:
        case OpPop:
            DEVPRINT("  %s %s\n", op2str(op), reg2str(dest));
            break;
        case OpLoad:
            DEVPRINT("  load %s, [%s]\n", reg2str(dest), reg2str(src1));
            break;
        case OpStore:
            DEVPRINT("  store [%s], %s\n", reg2str(dest), reg2str(src1));
            break;
        case OpJump:
            DEVPRINT("  jump %d\n", imme);
            break;
        case OpJZ:
            DEVPRINT("  jz %d\n", imme);
            break;
        case CPrintf:
            DEVPRINT("  print\n");
            break;
        default:
            panic("Invalid op code");
            break;
        }
    }
    DEVPRINT("%d lines of code\n", insNum);
}
// debug only
