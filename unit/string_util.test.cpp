#include <gtest/gtest.h>
#include <string>
#include "../cc.h"

TEST(string_util_test, sstream) {
    make_sstream(ss, 256);
    sstream_append(&ss, "hel");
    EXPECT_EQ(std::string(sstream_get(&ss)), "hel");
    sstream_append(&ss, "lo");
    EXPECT_EQ(std::string(sstream_get(&ss)), "hello");
    sstream_appendfmt(&ss, ", from Mr. %s and his %d friends", "Brown", 2);
    EXPECT_EQ(std::string(sstream_get(&ss)), "hello, from Mr. Brown and his 2 friends");

    sstream_clear(&ss);
    sstream_reserve(&ss, 64);
    sstream_push_back(&ss, 'a');
    sstream_push_back(&ss, 'b');
    sstream_push_back(&ss, 'c');
    EXPECT_EQ(std::string(sstream_get(&ss)), "abc");
    sstream_clear(&ss);
}

TEST(string_util_test, shortenpath) {
    // shortpath
    {
        char path[MAX_PATH_LEN] = "./file.h";
        shortenpath(path, sizeof(path));
        EXPECT_EQ(path, std::string("file.h"));
    }
    {
        char path[MAX_PATH_LEN] = "../file.h";
        shortenpath(path, sizeof(path));
        EXPECT_EQ(path, std::string("../file.h"));
    }
    {
        char path[MAX_PATH_LEN] = "../ab/..//./pp/pp23/./../ac/file.h";
        shortenpath(path, sizeof(path));
        EXPECT_EQ(path, std::string("../pp/ac/file.h"));
    }
}

TEST(string_util_test, path_concat) {
    {
        const char* c = "foo.c";
        struct slice_t h;
        h.start = "../bar.h";
        h.len = strlen(h.start);
        const char* path = path_concat(c, &h);
        EXPECT_EQ(path, std::string("../bar.h"));
    }
    {
        const char* c = "cc/unit/test.cpp";
        struct slice_t h;
        h.start = "../../cc.h";
        h.len = strlen(h.start);
        const char* path = path_concat(c, &h);
        EXPECT_EQ(path, std::string("cc.h"));
    }
}

#if 0


    {
        struct string_view sv;
        sv.start = "../bar.h";
        sv.len = strlen(sv.start);
        const char* path = filepath("../basepath/foo.c", &sv);
        expect_eq(strcmp(path, "../basepath/../bar.h"), 0);
        printf("path is \"%s\"\n", path);
    }

#endif