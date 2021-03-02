#include "unit_test.h"

static unsigned int hash_str(const char *str, int length) {
    unsigned int hash = 5381;
    for (int i = 0; i < length; ++i) {
        hash = ((hash << 5) + hash) + str[i];  // hash * 33 + c
    }

    return hash;
}

#include "../../code/containers/dict_decl.h"
#include "../../code/containers/dict_impl.h"

static unsigned int hash_func(const char *str) {
    return hash_str(str, (int)strlen(str));
}

static void print_dict_layout(IntDict *dict) {
    printf("dict has %d elements, %d capacity\n", dict->size, dict->capacity);
    for (int i = 0; i < dict->capacity; ++i) {
        printf("{ %2d:", i);
        for (IntDictNode *n = dict->bucket[i]; n; n = n->next) {
            printf(" %.*s ->", n->keyLen, n->key);
        }
        printf(" null }\n");
    }
    printf("--------------\n");
}

static void print_dict(IntDict *dict) {
    printf("dict has %d elements, %d capacity\n", dict->size, dict->capacity);
    for (int i = 0; i < dict->capacity; ++i) {
        for (IntDictNode *n = dict->bucket[i]; n; n = n->next) {
            printf("{ %.*s : %d }\n", n->keyLen, n->key, n->data);
        }
    }
    printf("--------------\n");
}

int main() {
    TEST_BEGIN("dict");

    // hash function
    EXPECT_EQ(hash_func("define") % TC_DICT_INITIAL_CAP, hash_func("int") % TC_DICT_INITIAL_CAP);
    EXPECT_EQ(hash_func("ifndef") % TC_DICT_INITIAL_CAP, hash_func("dummy") % TC_DICT_INITIAL_CAP);

    IntDict *dict = int_dict_make();

    int_dict_insert(dict, "dummy", 1);
    int_dict_insert(dict, "dummy2", 2);
    int_dict_insert(dict, "dummy3", 3);

    EXPECT_EQ(int_dict_has(dict, "dummy"), true);
    EXPECT_EQ(int_dict_has(dict, "dummy1"), false);
    EXPECT_EQ(int_dict_has(dict, "dummy2"), true);
    EXPECT_EQ(dict->size, 3);
    EXPECT_EQ(dict->capacity, TC_DICT_INITIAL_CAP);

    int i;
    EXPECT_EQ(int_dict_get(dict, "dummy", &i), true);
    EXPECT_EQ(i, 1);

    int_dict_insert(dict, "dummy2", 1212);
    EXPECT_EQ(dict->size, 3);
    int_dict_get(dict, "dummy2", &i);
    EXPECT_EQ(i, 1212);

    int_dict_insert(dict, "dummy", 0);
    int_dict_insert(dict, "char", 22);
    int_dict_insert(dict, "string", 333);
    int_dict_insert(dict, "this is a super long test", 4444);
    int_dict_insert(dict, "define", 55555);
    int_dict_insert(dict, "ifndef", 666666);
    int_dict_insert(dict, "register", 3);
    int_dict_insert(dict, "float", 6);
    int_dict_insert(dict, "int", 7);

    EXPECT_EQ(int_dict_erase(dict, "int"), true);
    EXPECT_EQ(int_dict_erase(dict, "int"), false);
    EXPECT_EQ(int_dict_erase(dict, "define"), true);

    int_dict_insert(dict, "auto", 8);

    EXPECT_EQ(dict->capacity, TC_DICT_INITIAL_CAP);

    // print_dict(dict);

    int_dict_clear(dict);
    int_dict_destroy(&dict);

    TEST_END()
}
