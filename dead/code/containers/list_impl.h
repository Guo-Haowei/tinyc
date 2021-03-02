#include "config.h"

T_NAME(List) * T_FUNC_NAME(list_make)() {
    T_NAME(List) *list = CALLOC(sizeof(T_NAME(List)));
    list->begin = NULL;
    list->end = NULL;
    list->size = 0;
    return list;
}

void T_FUNC_NAME(list_clear)(T_NAME(List) * list) {
    assert(list);

    while (list->size) {
        T_FUNC_NAME(list_pop_back)
        (list);
    }
}

void T_FUNC_NAME(list_destroy)(T_NAME(List) * *pList) {
    assert(pList);

    T_FUNC_NAME(list_clear)
    (*pList);
    FREE(*pList);
    *pList = NULL;
}

T T_FUNC_NAME(list_front)(T_NAME(List) * list) {
    assert(list);
    return list->begin->data;
}

T T_FUNC_NAME(list_back)(T_NAME(List) * list) {
    assert(list);
    return list->end->data;
}

T T_FUNC_NAME(list_at)(T_NAME(List) * list, int n) {
    assert(list);
    assert(list->size > n);

    T_NAME(ListNode) *node = list->begin;

    unsigned int i = 0u;
    /// OPTIMIZE: based on n, decide iterate from front or back
    for (T_NAME(ListNode) *node = list->begin; node; node = node->next, ++i) {
        if (n == i) {
            return node->data;
        }
    }

    // should never reach here
    assert(0);
    return (T)0;
}

void T_FUNC_NAME(list_push_front)(T_NAME(List) * list, T data) {
    assert(list);

    T_NAME(ListNode) *node = CALLOC(sizeof(T_NAME(ListNode)));
    node->prev = NULL;
    node->data = data;
    node->next = list->begin;
    if (0 == list->size) {
        list->end = node;
    } else {
        list->begin->prev = node;
    }
    list->begin = node;
    ++list->size;
}

void T_FUNC_NAME(list_push_back)(T_NAME(List) * list, T data) {
    assert(list);

    T_NAME(ListNode) *node = CALLOC(sizeof(T_NAME(ListNode)));
    node->next = NULL;
    node->data = data;
    node->prev = list->end;
    if (0 == list->size) {
        list->begin = node;
    } else {
        list->end->next = node;
    }
    list->end = node;
    ++list->size;
}

T T_FUNC_NAME(list_pop_front)(T_NAME(List) * list) {
    assert(list);
    assert(list->size);
    assert(list->begin);
    assert(list->end);

    T_NAME(ListNode) *first = list->begin;

    list->begin = first->next;
    if (first->next) {
        first->next->prev = NULL;
    } else {
        list->end = NULL;
    }

    --list->size;
    T ret = first->data;
    FREE(first);

    return ret;
}

T T_FUNC_NAME(list_pop_back)(T_NAME(List) * list) {
    assert(list);
    assert(list->size);
    assert(list->begin);
    assert(list->end);

    T_NAME(ListNode) *last = list->end;

    list->end = last->prev;
    if (last->prev) {
        last->prev->next = NULL;
    } else {
        list->begin = NULL;
    }

    --list->size;
    T ret = last->data;
    FREE(last);

    return ret;
}

T T_FUNC_NAME(list_erase)(T_NAME(List) * list, int n) {
    assert(list);
    assert(list->size > n);

    if (n == 0) {
        return T_FUNC_NAME(list_pop_front)(list);
    }

    if (n == list->size - 1) {
        return T_FUNC_NAME(list_pop_back)(list);
    }

    unsigned int i = 1u;
    for (T_NAME(ListNode) *node = list->begin->next; node->next; node = node->next, ++i) {
        if (n == i) {
            T ret = node->data;

            node->prev->next = node->next;
            node->next->prev = node->prev;

            FREE(node);
            --list->size;
            return ret;
        }
    }

    assert(0);
    return (T)0;
}
