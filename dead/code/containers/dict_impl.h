#include "config.h"

void T_FUNC_NAME(dict_clear)(T_NAME(Dict) * dict) {
    assert(dict);
    assert(dict->bucket);

    for (int i = 0; i < dict->capacity; ++i) {
        T_NAME(DictNode) *cursor = dict->bucket[i];
        while (cursor) {
            T_NAME(DictNode) *tmp = cursor->next;
            FREE(cursor);
            cursor = tmp;
        }
        dict->bucket[i] = NULL;
    }

    dict->size = 0;
}

T_NAME(Dict) * T_FUNC_NAME(dict_make)() {
    T_NAME(Dict) *dict = CALLOC(sizeof(T_NAME(Dict)));
    dict->capacity = TC_DICT_INITIAL_CAP;
    dict->size = 0;
    int bucketSize = TC_DICT_INITIAL_CAP * sizeof(void *);
    dict->bucket = CALLOC(bucketSize);

    return dict;
}

void T_FUNC_NAME(dict_destroy)(T_NAME(Dict) * *pDict) {
    assert(pDict);
    assert(*pDict);

    T_NAME(Dict) *dict = *pDict;
    T_FUNC_NAME(dict_clear)
    (dict);

    FREE(dict->bucket);
    dict->bucket = NULL;
    FREE(dict);
    *pDict = NULL;
}

bool T_FUNC_NAME(dict_nhas)(T_NAME(Dict) const *dict, char const *key, int keyLen) {
    assert(dict);
    assert(key);

    unsigned int hash = hash_str(key, keyLen);
    T_NAME(DictNode) *n = dict->bucket[hash % dict->capacity];
    for (; n; n = n->next) {
        if (keyLen == n->keyLen && strncmp(key, n->key, keyLen) == 0) {
            return true;
        }
    }

    return false;
}

bool T_FUNC_NAME(dict_nget)(T_NAME(Dict) * dict, char const *key, int keyLen, T *out) {
    assert(dict);
    assert(key);

    unsigned int hash = hash_str(key, keyLen);
    T_NAME(DictNode) *n = dict->bucket[hash % dict->capacity];
    for (; n; n = n->next) {
        if (keyLen == n->keyLen && strncmp(key, n->key, keyLen) == 0) {
            *out = n->data;
            return true;
        }
    }

    return false;
}

void T_FUNC_NAME(dict_ninsert)(T_NAME(Dict) * dict, char const *key, int keyLen, T value) {
    assert(dict);
    assert(key);

    unsigned int hash = hash_str(key, keyLen);

    for (T_NAME(DictNode) *node = dict->bucket[hash % dict->capacity]; node; node = node->next) {
        if (keyLen == node->keyLen && strncmp(key, node->key, keyLen) == 0) {
            node->data = value;
            return;
        }
    }

    T_NAME(DictNode) *node = CALLOC(sizeof(T_NAME(DictNode)));
    node->data = value;
    node->keyLen = keyLen;
    node->key = key;
    node->hash = hash;

    if (dict->size * 100 > TC_DICT_HIGH_WATERMARK * dict->capacity) {
        int cap = dict->capacity;
        dict->capacity *= 2;
        T_NAME(DictNode) **bucket = CALLOC(dict->capacity * sizeof(void *));

        for (int i = 0; i < cap; ++i) {
            T_NAME(DictNode) *node = dict->bucket[i];

            while (node) {
                T_NAME(DictNode) *next = node->next;
                int slot = node->hash % dict->capacity;
                node->next = bucket[slot];
                bucket[slot] = node;
                node = next;
            }
        }

        FREE(dict->bucket);
        dict->bucket = bucket;
    }

    unsigned int slot = hash % dict->capacity;
    node->next = dict->bucket[slot];
    dict->bucket[slot] = node;
    ++dict->size;
}

bool T_FUNC_NAME(dict_nerase)(T_NAME(Dict) * dict, char const *key, int keyLen) {
    assert(dict);
    assert(key);

    unsigned int slot = hash_str(key, keyLen) % dict->capacity;

    T_NAME(DictNode)
    tmp;
    tmp.next = dict->bucket[slot];

    for (T_NAME(DictNode) *a1 = &tmp, *a2 = tmp.next; a2; a1 = a2, a2 = a1->next) {
        if (keyLen == a2->keyLen && strncmp(key, a2->key, keyLen) == 0) {
            --dict->size;
            a1->next = a2->next;
            FREE(a2);
            dict->bucket[slot] = tmp.next;
            return true;
        }
    }

    return false;
}

/// TODO: provide my own strlen
bool T_FUNC_NAME(dict_has)(T_NAME(Dict) const *dict, char const *key) {
    return T_FUNC_NAME(dict_nhas)(dict, key, (int)strlen(key));
}

bool T_FUNC_NAME(dict_get)(T_NAME(Dict) * dict, char const *key, T *out) {
    return T_FUNC_NAME(dict_nget)(dict, key, (int)strlen(key), out);
}

void T_FUNC_NAME(dict_insert)(T_NAME(Dict) * dict, char const *key, T value) {
    T_FUNC_NAME(dict_ninsert)
    (dict, key, (int)strlen(key), value);
}

bool T_FUNC_NAME(dict_erase)(T_NAME(Dict) * dict, char const *key) {
    return T_FUNC_NAME(dict_nerase)(dict, key, (int)strlen(key));
}
