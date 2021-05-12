#include <gtest/gtest.h>
#include "../cc.h"

static void test_list(struct list_t* list, const size_t len, ...);

TEST(list_test, basic) {
    list_new(list);

    // size
    EXPECT_EQ(list_len(list), 0);
    EXPECT_EQ(list_empty(list), true);

    // insert and delete
    list_push_back(list, 1);
    list_push_front(list, -1);
    test_list(list, 2, -1, 1);

    list_push_back(list, 2);
    list_push_front(list, -2);
    test_list(list, 4, -2, -1, 1, 2);

    list_push_back(list, 3);
    list_push_front(list, -3);
    test_list(list, 6, -3, -2, -1, 1, 2, 3);

    list_push_back(list, 4);
    list_push_front(list, -4);
    test_list(list, 8, -4, -3, -2, -1, 1, 2, 3, 4);

    // list_pop_front
    int target_len = list->len - 3;
    while (list->len > target_len) {
        list_pop_front(size_t, list);
    }
    test_list(list, target_len, -1, 1, 2, 3, 4);

    // list_pop_back
    target_len = list->len - 2;
    while (list->len > target_len) {
        list_pop_back(size_t, list);
    }
    test_list(list, target_len, -1, 1, 2);

    // list_clear
    EXPECT_FALSE(list_empty(list));
    list_clear(list);
    EXPECT_EQ(list_len(list), 0);

    list_push_back(list, 7);
    list_push_back(list, 6);
    list_push_front(list, 5);
    test_list(list, 3, 5, 7, 6);

    list_delete(list);
}

static void test_list(struct list_t* list, size_t len, ...) {
    int values[1024];
    assert(len <= ARRAY_LEN(values));

    // list_len
    EXPECT_EQ(list_len(list), len);

    va_list args;
    va_start(args, len);
    for (int i = 0; i < len; ++i) {
        values[i] = va_arg(args, size_t);
    }
    va_end(args);

    // list_front, list_back
    EXPECT_EQ(list_front(size_t, list), values[0]);
    EXPECT_EQ(list_back(size_t, list), values[len - 1]);

    // list_at
    for (int i = 0; i < len; ++i) {
        EXPECT_EQ(list_at(size_t, list, i), values[i]);
    }

    // list_node_t
    int i = 0;
    for (struct list_node_t* it = list->front; it; it = it->next) {
        EXPECT_EQ((size_t)it->data, values[i]);
        ++i;
    }

    i = len - 1;
    for (struct list_node_t* it = list->back; it; it = it->prev) {
        EXPECT_EQ((size_t)it->data, values[i]);
        --i;
    }
}
