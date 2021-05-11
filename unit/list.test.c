#include "../cc.h"
#include "unit.h"

static void test_list(struct list_t* list, const int len, ...);

int main() {
    test_begin("list");
    list_new(list);

    expect_eq(list_len(list), 0);
    expect_eq(list_empty(list), true);

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
    assert(target_len >= 0);
    while (list->len > target_len) {
        list_pop_front(int, list);
    }
    test_list(list, target_len, -1, 1, 2, 3, 4);

    // list_pop_back
    target_len = list->len - 2;
    assert(target_len >= 0);
    while (list->len > target_len) {
        list_pop_back(int, list);
    }
    test_list(list, target_len, -1, 1, 2);

    // list_clear
    expect_eq(list_empty(list), false);
    list_clear(list);
    expect_eq(list_len(list), 0);

    list_push_back(list, 7);
    list_push_back(list, 6);
    list_push_front(list, 5);
    test_list(list, 3, 5, 7, 6);

    list_delete(list);
    test_end();
}

static void test_list(struct list_t* list, const int len, ...) {
    int values[1024];
    assert(len <= ARRAY_LEN(values));

    // list_len
    expect_eq(list_len(list), len);

    va_list args;
    va_start(args, len);
    for (int i = 0; i < len; ++i) {
        values[i] = va_arg(args, int);
    }
    va_end(args);

    // list_front, list_back
    expect_eq(list_front(int, list), values[0]);
    expect_eq(list_back(int, list), values[len - 1]);

    // list_at
    for (int i = 0; i < len; ++i) {
        expect_eq(list_at(int, list, i), values[i]);
    }

    // list_node_t
    int i = 0;
    for (struct list_node_t* it = list->front; it; it = it->next) {
        expect_eq((int)it->data, values[i]);
        ++i;
    }
    assert(i == len);
    i = len - 1;
    for (struct list_node_t* it = list->back; it; it = it->prev) {
        expect_eq((int)it->data, values[i]);
        --i;
    }
    assert(i == -1);
}
