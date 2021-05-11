#include "../cc.h"
#include "unit.h"

int main() {
    test_begin("string_util");

    // string_buf
    {
        make_string_buf(sb);
        sb_clear(&sb);
    }

    // shortpath
    {
        const char* path = shortpath("./");
        expect_eq(strcmp(path, "./"), 0);
    }
    {
        const char* path = shortpath("/");
        expect_eq(strcmp(path, "./"), 0);
    }
    {
        const char* path = shortpath("..");
        expect_eq(strcmp(path, ".."), 0);
    }
    {
        const char* path = shortpath("../ab/..//./pp/pp23/./../ac/file.h");
        // expect_eq(strcmp(path, "../../.h"), 0);
    }

#if 0
    // filepath
    {
        struct string_view sv;
        sv.start = "../bar.h";
        sv.len = strlen(sv.start);
        const char* path = filepath("foo.c", &sv);
        expect_eq(strcmp(path, "../bar.h"), 0);
        printf("path is \"%s\"\n", path);
    }
    {
        struct string_view sv;
        sv.start = "../bar.h";
        sv.len = strlen(sv.start);
        const char* path = filepath("../basepath/foo.c", &sv);
        expect_eq(strcmp(path, "../basepath/../bar.h"), 0);
        printf("path is \"%s\"\n", path);
    }
#endif

    // expect_eq(list_len(list), 0);
    // expect_eq(list_empty(list), true);

    // list_push_back(list, 1);
    // list_push_front(list, -1);

    // test_list(list, 2, -1, 1);

    // list_push_back(list, 2);
    // list_push_front(list, -2);
    // test_list(list, 4, -2, -1, 1, 2);

    // list_push_back(list, 3);
    // list_push_front(list, -3);
    // test_list(list, 6, -3, -2, -1, 1, 2, 3);

    // list_push_back(list, 4);
    // list_push_front(list, -4);
    // test_list(list, 8, -4, -3, -2, -1, 1, 2, 3, 4);

    // // list_pop_front
    // int target_len = list->len - 3;
    // assert(target_len >= 0);
    // while (list->len > target_len) {
    //     list_pop_front(int, list);
    // }
    // test_list(list, target_len, -1, 1, 2, 3, 4);

    // // list_pop_back
    // target_len = list->len - 2;
    // assert(target_len >= 0);
    // while (list->len > target_len) {
    //     list_pop_back(int, list);
    // }
    // test_list(list, target_len, -1, 1, 2);

    // // list_clear
    // expect_eq(list_empty(list), false);
    // list_clear(list);
    // expect_eq(list_len(list), 0);

    // list_push_back(list, 7);
    // list_push_back(list, 6);
    // list_push_front(list, 5);
    // test_list(list, 3, 5, 7, 6);

    // list_delete(list);
    test_end();
}