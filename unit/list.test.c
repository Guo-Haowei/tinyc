#include "../cc.h"

void list_print(struct list_t* list) {
    printf("\tsize: %d\n", list->len);
    printf("\t[ ");
    for (struct list_node_t* n = list->front; n; n = n->next) {
        printf("%d ", (int)(n->data));
    }
    printf("] reversed [ ");
    for (struct list_node_t* n = list->back; n; n = n->prev) {
        printf("%d ", (int)(n->data));
    }
    printf("]\n");
}

int main() {
    printf("****************************************************************\n");
    printf("[list]\n");
    list_new(list);

    printf("push [ -1 1 ]\n");
    list_push_back(list, 1);
    list_push_front(list, -1);

    list_print(list);
    printf("push [ -2 2 ]\n");
    list_push_back(list, 2);
    list_push_front(list, -2);
    list_print(list);
    printf("push [ -3 3 ]\n");
    list_push_back(list, 3);
    list_push_front(list, -3);
    list_print(list);
    printf("push [ -4 4 ]\n");
    list_push_back(list, 4);
    list_push_front(list, -4);
    list_print(list);

    /// TODO: unit test frame work
    assert(list_at(int, list, 0) == -4);
    assert(list_at(int, list, 1) == -3);
    assert(list_at(int, list, 2) == -2);
    assert(list_at(int, list, 3) == -1);
    assert(list_at(int, list, 4) == +1);
    assert(list_at(int, list, 5) == +2);
    assert(list_at(int, list, 6) == +3);
    assert(list_at(int, list, 7) == +4);

    const int half = list->len / 2;
    while (list->len > half) {
        printf("pop %d from front\n", list_pop_front(int, list));
        printf("pop %d from back\n", list_pop_back(int, list));
        list_print(list);
    }
    while (list->len >= 2) {
        printf("pop %d from back\n", list_pop_back(int, list));
        printf("pop %d from front\n", list_pop_front(int, list));
        list_print(list);
    }

    list_delete(list);
    printf("****************************************************************\n");
}

