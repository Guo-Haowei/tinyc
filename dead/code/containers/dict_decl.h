#include "config.h"

#define TC_DICT_INITIAL_CAP 16
#define TC_DICT_HIGH_WATERMARK 70

typedef struct T_NAME(DictNode) {
    char const *key;
    int keyLen;
    unsigned int hash;
    T data;
    struct T_NAME(DictNode) * next;
} T_NAME(DictNode);

typedef struct
{
    int capacity;
    int size;
    T_NAME(DictNode) * *bucket;
} T_NAME(Dict);

T_NAME(Dict) * T_FUNC_NAME(dict_make)();

void T_FUNC_NAME(dict_clear)(T_NAME(Dict) * dict);

void T_FUNC_NAME(dict_destroy)(T_NAME(Dict) * *pDict);

bool T_FUNC_NAME(dict_nhas)(T_NAME(Dict) const *dict, char const *key, int keyLen);

bool T_FUNC_NAME(dict_nget)(T_NAME(Dict) * dict, char const *key, int keyLen, T *out);

void T_FUNC_NAME(dict_ninsert)(T_NAME(Dict) * dict, char const *key, int keyLen, T value);

bool T_FUNC_NAME(dict_nerase)(T_NAME(Dict) * dict, char const *key, int keyLen);

bool T_FUNC_NAME(dict_has)(T_NAME(Dict) const *dict, char const *key);

bool T_FUNC_NAME(dict_get)(T_NAME(Dict) * dict, char const *key, T *out);

void T_FUNC_NAME(dict_insert)(T_NAME(Dict) * dict, char const *key, T value);

bool T_FUNC_NAME(dict_erase)(T_NAME(Dict) * dict, char const *key);
