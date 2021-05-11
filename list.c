#include "cc.h"

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

static struct list_node_t* _list_node_new();

struct list_t* _list_new() {
    struct list_t* list = malloc(sizeof(struct list_t));
    list->front = NULL;
    list->back = NULL;
    list->len = 0;
    return list;
}

void _list_delete(struct list_t** plist) {
    cassert(plist && *plist);
    _list_clear(*plist);
    free(*plist);
    *plist = NULL;
}

void _list_clear(struct list_t* list) {
    cassert(list);

    while (list->len) {
        _list_pop_back(list);
    }

    cassert(list->len == 0);
    cassert(list->front == 0);
    cassert(list->back == 0);
}

void* _list_back(struct list_t* list) {
    cassert(list && list->len && list->front && list->back);
    return list->back->data;
}

void* _list_front(struct list_t* list) {
    cassert(list && list->len && list->front && list->back);
    return list->front->data;
}

void* _list_at(struct list_t* list, int idx) {
    cassert(list && idx >= 0 && idx < list->len);

    struct list_node_t* n = list->front;
    for (; idx--; n = n->next)
        ;
    return n->data;
}

void _list_push_front(struct list_t* list, void* data) {
    cassert(list);
    struct list_node_t* n = _list_node_new();
    n->prev = NULL;
    n->next = list->front;
    n->data = data;
    if (list->len == 0) {
        list->back = n;
    } else {
        list->front->prev = n;
    }

    list->front = n;
    ++list->len;
}

void _list_push_back(struct list_t* list, void* data) {
    cassert(list);
    struct list_node_t* n = _list_node_new();
    n->next = NULL;
    n->prev = list->back;
    n->data = data;
    if (list->len == 0) {
        list->front = n;
    } else {
        list->back->next = n;
    }

    list->back = n;
    ++list->len;
}

void* _list_pop_front(struct list_t* list) {
    cassert(list && list->len && list->front && list->back);
    struct list_node_t* n = list->front;
    list->front = n->next;
    if (n->next) {
        n->next->prev = NULL;
    } else {
        list->back = NULL;
    }

    --list->len;
    void* ret = n->data;
    free(n);
    return ret;
}

void* _list_pop_back(struct list_t* list) {
    cassert(list && list->len && list->front && list->back);
    struct list_node_t* n = list->back;
    list->back = n->prev;
    if (n->prev) {
        n->prev->next = NULL;
    } else {
        list->front = NULL;
    }

    --list->len;
    void* ret = n->data;
    free(n);
    return ret;
}

struct list_node_t* _list_node_new() {
    struct list_node_t* node = malloc(sizeof(struct list_node_t));
    node->prev = NULL;
    node->next = NULL;
    node->data = NULL;
    return node;
}

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
