// debug only

char* op2str(int op) {
    switch (op) {
        case OP_RET: return "ret";
        case OP_MOV: return "mov";
        case OP_ADD: return "add";
        case OP_SUB: return "sub";
        case OP_MUL: return "mul";
        case OP_DIV: return "div";
        case OP_REM: return "rem";
        case OP_PUSH: return "push";
        case OP_POP: return "pop";
        case OP_EQ: return "EQ";
        case OP_NE: return "NE";
        case OP_LT: return "LT";
        case OP_LE: return "LE";
        case OP_GT: return "GT";
        case OP_GE: return "GE";
        default: panic("unknown token"); return "<error>";
    }
}

char* reg2str(int reg) {
    switch (reg) {
        case EAX: return "eax";
        case EBX: return "ebx";
        case ECX: return "ecx";
        case EDX: return "edx";
        case ESP: return "esp";
        case EBP: return "ebp";
        default: printf("%d\n", reg);panic("unknown reg"); return "<error>";
    }
}

void dump_tk(int i) {
    DEVPRINT("[ %-7s] ln:%d: [%.*s]\n", tk2str(g_tks[i].kind), g_tks[i].ln, g_tks[i].end - g_tks[i].start, g_tks[i].start);
}

void dump_tks() {
    for (int i = 0; i < g_tkCnt; ++i) {
        dump_tk(i);
    }
}

void dump_code() {
    for (int pc = 0; pc < g_insCnt; ++pc) {
        int op = g_instructs[pc].op;
        int imme = g_instructs[pc].imme;
        int dest = (op & 0xFF00) >> 8;
        int src1 = (op & 0xFF0000) >> 16;
        int src2 = (op & 0xFF000000) >> 24;
        op = op & 0xFF;
        DEVPRINT("[ %4d ] ", pc);
        switch (op) {
        case OP_MOV:
            DEVPRINT("  mov %s, ", reg2str(dest));
            if (src2 == IMME) DEVPRINT("%d\n", imme);
            else DEVPRINT("%s\n", reg2str(src2));
            break;
        case OP_RET: 
            DEVPRINT("  ret\n");
            break;
        case OP_ADD: 
        case OP_SUB: 
        case OP_MUL: 
        case OP_DIV: 
        case OP_REM: 
            DEVPRINT("  %s %s, %s, ", op2str(op), reg2str(dest), reg2str(src1));
            if (src2 == IMME) DEVPRINT("%d\n", imme);
            else DEVPRINT("%s\n", reg2str(src2));
            break;
        case OP_EQ:
        case OP_NE:
        case OP_GT:
        case OP_GE:
        case OP_LT:
        case OP_LE:
            DEVPRINT("  %s %s, %s, %s\n", op2str(op), reg2str(dest), reg2str(src1), reg2str(src2));
            break;
        case OP_PUSH:
            if (src2 == IMME) DEVPRINT("  %s %d\n", op2str(op), imme);
            else DEVPRINT("  %s %s\n", op2str(op), reg2str(src2));
            break;
        case OP_POP:
            DEVPRINT("  %s %s\n", op2str(op), reg2str(dest));
            break;
        case OP_LOAD:
            DEVPRINT("  load %s, [%s]\n", reg2str(dest), reg2str(src1));
            break;
        case OP_STORE:
            DEVPRINT("  store [%s], %s\n", reg2str(dest), reg2str(src1));
            break;
        case OP_JUMP:
            DEVPRINT("  jump %d\n", imme);
            break;
        case OP_JZ:
            DEVPRINT("  jz %d\n", imme);
            break;
        case OP_CALL:
            DEVPRINT("  call %d\n", imme);
            break;
        case OP_PRINTF:
            DEVPRINT("  print\n");
            break;
        default:
            panic("Invalid op code");
            break;
        }
    }
    DEVPRINT("%d lines of code\n", g_insCnt);
}
// debug only
