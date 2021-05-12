#include "cc.h"

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

struct map_t* map_new() {
    struct map_t* map = malloc(sizeof(struct map_t));
    map->list = list_new();
    return map;
}

void map_delete(struct map_t* map) {
    cassert(map);
    for (struct list_node_t* it = map->list->front; it; it = it->next) {
        free(it->data);
    }
    list_delete(map->list);
    free(map);
}

bool _map_try_insert(struct map_t* map, const char* key, void* data) {
    struct map_pair_t* it = map_find(map, key);
    if (it) {
        return false;
    }

    struct map_pair_t* node = malloc(sizeof(struct map_pair_t));
    node->key = key;
    node->data = data;
    list_push_back(map->list, node);
}

void _map_insert(struct map_t* map, const char* key, void* data) {
    struct map_pair_t* it = map_find(map, key);
    if (it) {
        /// TODO: fix this
        cassert("collision!!!\n");
        it->data = data;
        return;
    }

    struct map_pair_t* node = malloc(sizeof(struct map_pair_t));
    node->key = key;
    node->data = data;
    list_push_back(map->list, node);
}

struct map_pair_t* map_find(struct map_t* map, const char* key) {
    for (struct list_node_t* it = map->list->front; it; it = it->next) {
        struct map_pair_t* pair = it->data;
        if (streq(pair->key, key)) {
            return pair;
        }
    }

    return NULL;
}

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
