#include "cc.h"

struct DataType* g_char;
struct DataType* g_const_char;
struct DataType* g_int;
struct DataType* g_const_int;
struct DataType* g_void;
struct DataType* g_const_char_ptr;

struct DataType* datatype_new(int kind, int size, bool mut) {
    struct DataType* dt = alloct(sizeof(struct DataType));
    dt->kind = kind;
    dt->size = size;
    dt->mut = mut;
    dt->arrLen = 0;
    dt->base = NULL;
    return dt;
}

#define NEW_DATATYPE(KIND, SIZE, MUT, NAME)    \
    NAME = calloc(1, sizeof(struct DataType)); \
    NAME->kind = KIND

void init_dt_global() {
    NEW_DATATYPE(DT_CHAR, 1, true, g_char);
    NEW_DATATYPE(DT_CHAR, 1, false, g_const_char);

    NEW_DATATYPE(DT_INT, INT_SIZE_IN_BYTE, true, g_int);
    NEW_DATATYPE(DT_INT, INT_SIZE_IN_BYTE, false, g_const_int);

    NEW_DATATYPE(DT_VOID, 0, true, g_void);

    NEW_DATATYPE(DT_PTR, PTR_SIZE_IN_BYTE, true, g_const_char_ptr);
    g_const_char_ptr->base = g_const_char;
}

void free_dt_global() {
    cassert(g_char);
    cassert(g_const_char);
    cassert(g_int);
    cassert(g_const_int);
    cassert(g_void);
    cassert(g_const_char_ptr);

    free(g_char);
    free(g_const_char);

    free(g_int);
    free(g_const_int);

    free(g_void);

    free(g_const_char_ptr);
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
