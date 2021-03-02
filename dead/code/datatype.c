#include "toyc.h"

extern expect_and_pop_char_punct(TokenList *tokenList, char c);

DataType *make_empty_type() {
    DataType *dt = CALLOC(sizeof(DataType));
    return dt;
}

#define MAX_DATATYPE_DEPTH 16
const char *data_type_to_string(const DataType *type) {
    static char s_buffer[256];
    const DataType *typeCache[MAX_DATATYPE_DEPTH];
    int typeCnt = 0;
    for (const DataType *it = type; it; it = it->base) {
        typeCache[typeCnt++] = it;
    }

    int offset = 0;
    for (int i = typeCnt - 1; i >= 0; --i) {
        const DataType *type = typeCache[i];
        if (type->kind == DT_INT) {
            offset += snprintf(s_buffer + offset, sizeof(s_buffer) - offset, "int");
        } else if (type->kind == DT_VOID) {
            offset += snprintf(s_buffer + offset, sizeof(s_buffer) - offset, "void");
        } else if (type->kind == DT_PTR) {
            offset += snprintf(s_buffer + offset, sizeof(s_buffer) - offset, "*");
        } else if (type->kind == DT_ARRAY) {
            offset += snprintf(s_buffer + offset, sizeof(s_buffer) - offset, "[%d]", type->arraySize);
        } else {
            assertfmt(false, "Unable to print type");
        }
    }

    return s_buffer;
}

int data_type_size(const DataType *type) {
    assert(type);

    eDataType kind = type->kind;
    if (kind == DT_INT) {
        return INT_SIZE_IN_BYTE;
    } else if (kind == DT_PTR) {
        return POINTER_SIZE_IN_BYTE;
    } else if (kind == DT_ARRAY) {
        assert(type->arraySize);
        return type->arraySize * data_type_size(type->base);
    } else {
        /// TODO better error handling
        assert(0);
        return 0;
    }
}

const DataType *parse_data_type(TokenList *tokenList) {
    DataType *datatype = NULL;
    Token *tk = token_list_front(tokenList);
    if (match_str(tk, "int")) {
        token_list_pop_front(tokenList);
        datatype = make_empty_type();
        datatype->kind = DT_INT;
    } else {
        return NULL;
    }

    tk = token_list_front(tokenList);
    if (datatype && match_char(tk, '*')) {
        DataType *pointer = make_empty_type();
        pointer->kind = DT_PTR;
        pointer->base = datatype;
        datatype = pointer;
        token_list_pop_front(tokenList);
    }

    // skip variable name and parse potential array []
    if (datatype && token_list_front(tokenList)->kind == TK_ID) {
        Token *varName = token_list_pop_front(tokenList);
        if (match_char(token_list_front(tokenList), '[')) {
            token_list_pop_front(tokenList);

            DataType *arr = make_empty_type();
            arr->kind = DT_ARRAY;
            arr->base = datatype;
            datatype = arr;

            /// NOTE: only allow number
            tk = token_list_front(tokenList);
            if (!match_char(tk, ']')) {
                if (tk->kind == TK_NUMBER) {
                    arr->arraySize = atoi(tk->loc.p);
                    token_list_pop_front(tokenList);
                } else {
                    log_error_at_token("expect valid array size", tk);
                }
            }
            expect_and_pop_char_punct(tokenList, ']');
        }

        // push variable back
        token_list_push_front(tokenList, varName);
    }

    const char *typeHash = data_type_to_string(datatype);

    DataType *cache = NULL;

    // array could have it's size later on
    if (datatype->kind != DT_ARRAY && datatype_dict_get(s_dataTypeCache, typeHash, &cache)) {
        /// TODO: free datatype
        datatype = cache;
    }

    return datatype;
}
