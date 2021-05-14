#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "../cc.h"

static void print_map(struct map_t* map) {
    printf("map has size %d\n", (int)map->size);
    printf("map has cap %d\n", (int)map->cap);

    for (size_t i = 0; i < map->cap; ++i) {
        struct map_pair_t* pair = map->bucket + i;
        if (pair->key == NULL) {
            printf("{ null }\n");
            continue;
        }

        printf("{ \"%s\" : %d }\n", pair->key, int(size_t(pair->data)));
    }
}

TEST(map_test, basic) {
    struct map_t* map = map_new();

    // insert
    EXPECT_EQ(map_find(map, "int"), nullptr);

    map_insert(map, "int", 1);
    struct map_pair_t* it = map_find(map, "int");
    EXPECT_EQ(size_t(it->data), 1);

    it = map_find(map, "char");
    EXPECT_EQ(it, nullptr);

    map_insert(map, "char", 2);
    it = map_find(map, "char");
    EXPECT_EQ(size_t(it->data), 2);

    EXPECT_EQ(map->size, 2);
    map_insert(map, "int", 3);
    EXPECT_EQ(map->size, 2);

    it = map_find(map, "int");
    EXPECT_EQ(size_t(it->data), 3);

    map_delete(map);
}

TEST(map_test, keywords) {
    struct map_t* map = map_new();

    const char* kw[] = {
        "int",
        "const",
        "extern",
        "main",
        "for",
        "while",
        "do",
        "break",
        "continue",
        "return",
        "unsigned",
        "signed",
        "short",
        "char",
        "fib",
        "hello",
        "ascii",
        "77",
        "123",
        "543",
        "__",
        "799",
        "542",
        "177",
        "1123",
        "1543",
        "1__",
        "9799",
        "0A0F",
        "231",
        "3",
        "9",
        "_9",
        "9-99",
        "0542",
    };
    for (size_t i = 0; i < ARRAY_LEN(kw); ++i) {
        map_insert(map, kw[i], i);
    }

    for (size_t i = 0; i < ARRAY_LEN(kw); ++i) {
        struct map_pair_t* pair = map_find(map, kw[i]);
        EXPECT_TRUE(pair);
    }

    print_map(map);
    map_delete(map);
}
