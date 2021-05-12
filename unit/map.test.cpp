#include <gtest/gtest.h>
#include <string>
#include "../cc.h"

TEST(map_test, basic) {
    struct map_t* map = map_new();

    EXPECT_TRUE(map_empty(map));

    // insert
    map_insert(map, "int", "INT");
    struct map_pair_t* it = map_find(map, "int");
    EXPECT_EQ((const char*)it->data, std::string("INT"));

    it = map_find(map, "char");
    EXPECT_EQ(it, nullptr);

    map_insert(map, "char", "CHAR");
    it = map_find(map, "char");
    EXPECT_EQ((const char*)it->data, std::string("CHAR"));

    EXPECT_EQ(map_len(map), 2);
    map_insert(map, "int", "InT");
    EXPECT_EQ(map_len(map), 2);

    it = map_find(map, "int");
    EXPECT_EQ((const char*)it->data, std::string("InT"));

    map_delete(map);
}
