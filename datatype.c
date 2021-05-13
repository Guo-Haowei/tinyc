#include "cc.h"

struct DataType* datatype_new(int kind, bool mut) {
    struct DataType* dt = alloct(sizeof(struct DataType));
    dt->kind = kind;
    dt->mut = mut;
    dt->sizeInByte = 0;
    dt->base = NULL;
    return dt;
}

const char* datatype_string(const struct DataType* dt) {
    cassert(dt);

    const struct DataType* tmptype[12];
    size_t count = 0;
    for (; dt; dt = dt->base) {
        tmptype[count++] = dt;
    }

    static char buf[4096];
    char* p = buf;
    for (int idx = count - 1; idx >= 0; --idx) {
        const struct DataType* dt = tmptype[idx];
        switch (dt->kind) {
            case DT_PTR:
                p += sprintf(p, " *%s", dt->mut ? "" : "const");
                break;
            case DT_VOID:
            case DT_CHAR:
            case DT_INT:
                p += sprintf(p, "%s%s", dt->mut ? "" : "const ", dt2str(dt->kind));
                break;
            default:
                panic("TODO: implement datatype_print");
                break;
        }
    }

    buf[sizeof(buf) - 1] = '\0';
    return buf;
}
