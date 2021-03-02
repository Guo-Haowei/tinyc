#include "config.h"

typedef struct T_NAME(ListNode) {
    T data;
    struct T_NAME(ListNode) * prev;
    struct T_NAME(ListNode) * next;
} T_NAME(ListNode);

typedef struct
{
    int size;
    T_NAME(ListNode) * begin;
    T_NAME(ListNode) * end;
} T_NAME(List);

T_NAME(List) * T_FUNC_NAME(list_make)();

void T_FUNC_NAME(list_clear)(T_NAME(List) * list);

void T_FUNC_NAME(list_destroy)(T_NAME(List) * *pList);

T T_FUNC_NAME(list_front)(T_NAME(List) * list);

T T_FUNC_NAME(list_back)(T_NAME(List) * list);

T T_FUNC_NAME(list_at)(T_NAME(List) * list, int n);

void T_FUNC_NAME(list_push_front)(T_NAME(List) * list, T data);

void T_FUNC_NAME(list_push_back)(T_NAME(List) * list, T data);

T T_FUNC_NAME(list_pop_front)(T_NAME(List) * list);

T T_FUNC_NAME(list_pop_back)(T_NAME(List) * list);

T T_NAME(list_erase)(T_NAME(List) * list, int n);
