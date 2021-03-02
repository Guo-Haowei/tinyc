#include "toyc.h"

/// DataType table
#define T DataType*
#define T_NAME(x) DataType##x
#define T_FUNC_NAME(x) datatype_##x
#include "containers/dict_impl.h"
#undef T
#undef T_NAME
#undef T_FUNC_NAME

DataTypeDict* s_dataTypeCache = NULL;
DataType* s_intType = NULL;
DataType* s_intptrType = NULL;

void setup() {
    assert(!s_dataTypeCache);
    if (!s_dataTypeCache) {
        s_dataTypeCache = datatype_dict_make();
    }

    // int data
    s_intType = make_empty_type();
    s_intType->kind = DT_INT;
    datatype_dict_insert(s_dataTypeCache, "int", s_intType);

    // int pointer
    s_intptrType = make_empty_type();
    s_intptrType->kind = DT_INT;
    s_intptrType->base = s_intType;
    datatype_dict_insert(s_dataTypeCache, "int*", s_intptrType);
}
