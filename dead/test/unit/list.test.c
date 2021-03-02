#include "../../code/containers/list_decl.h"
#include "../../code/containers/list_impl.h"
#include "unit_test.h"

static void print_list(IntList *list) {
    printf("list has %d elements\n", list->size);
    printf("list has elements:");
    int size = 0;
    for (IntListNode *n = list->begin; n; n = n->next, ++size) {
        printf(" %d", n->data);
    }
    assert(size == list->size);
    printf("\n");

    printf("reversed list has elements:");
    size = 0;
    for (IntListNode *n = list->end; n; n = n->prev, ++size) {
        printf(" %d", n->data);
    }
    printf("\n--------------\n");
    assert(size == list->size);
}

int main() {
    TEST_BEGIN("list");

    IntList *list = int_list_make();

    int_list_push_back(list, 0);
    // 0
    int_list_push_back(list, 1);
    // 0, 1
    int_list_push_back(list, 2);
    // 0, 1, 2
    int_list_push_back(list, 3);
    // 0, 1, 2, 3

    EXPECT_EQ(list->size, 4);
    EXPECT_EQ(int_list_front(list), int_list_at(list, 0));
    EXPECT_EQ(int_list_back(list), int_list_at(list, 3));
    EXPECT_EQ(2, int_list_at(list, 2));
    EXPECT_EQ(3, int_list_at(list, 3));

    int_list_push_front(list, 99);
    // 99, 0, 1, 2, 3
    int_list_push_front(list, 88);
    // 88, 99, 0, 1, 2, 3
    int_list_push_front(list, 77);
    // 77, 88, 99, 0, 1, 2, 3
    EXPECT_EQ(list->size, 7);
    EXPECT_EQ(int_list_front(list), 77);
    EXPECT_EQ(int_list_at(list, 2), 99);

    EXPECT_EQ(int_list_pop_front(list), 77);
    // 88, 99, 0, 1, 2, 3
    EXPECT_EQ(int_list_pop_back(list), 3);
    // 88, 99, 0, 1, 2
    int_list_push_back(list, 66);
    // 88, 99, 0, 1, 2, 66
    EXPECT_EQ(int_list_at(list, list->size - 1), 66);

    EXPECT_EQ(int_list_erase(list, 2), 0);
    // 88, 99, 1, 2, 66
    EXPECT_EQ(int_list_erase(list, 0), 88);
    // 99, 1, 2, 66
    EXPECT_EQ(int_list_erase(list, list->size - 1), 66);
    // 99, 1, 2

    // clear
    int_list_clear(list);
    EXPECT_EQ(list->size, 0);

    // destroy
    int_list_destroy(&list);

    TEST_END()
}
