#include "cc.h"

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

enum {
    MAP_DEFAULT_CAPACITY = 17,
    MAP_HIGH_WATERMARK = 65
};

struct map_t* map_new() {
    struct map_t* map = malloc(sizeof(struct map_t));
    map->size = 0;
    map->cap = MAP_DEFAULT_CAPACITY;
    map->bucket = calloc(1, sizeof(struct map_pair_t) * MAP_DEFAULT_CAPACITY);
    return map;
}

void map_delete(struct map_t* map) {
    cassert(map);
    if (map->bucket) {
        free(map->bucket);
    }
    free(map);
}

struct map_pair_t* map_find(struct map_t* map, const char* key) {
    cassert(map);
    cassert(map->bucket);

    const uint hash = hash_str(key);
    uint slot = hash % map->cap;

    for (size_t i = 0; i < map->cap; ++i, slot = (slot + 1) % map->cap) {
        struct map_pair_t* pair = map->bucket + slot;
        if (pair->key && streq(key, pair->key)) {
            return pair;
        }
    }

    return NULL;
}

void _map_insert(struct map_t* map, const char* key, void* data) {
    cassert(map);
    cassert(map->bucket);
    cassert(key);

    struct map_pair_t* it = map_find(map, key);
    if (it) {
        it->data = data;
        return;
    }

    const uint hash = hash_str(key);
    uint slot = hash % map->cap;

    for (size_t i = 0; i < map->cap; ++i, slot = (slot + 1) % map->cap) {
        struct map_pair_t* pair = map->bucket + slot;
        if (pair->key == NULL) {
            pair->key = key;
            pair->hash = hash;
            pair->data = data;
            ++map->size;
            break;
        }
    }

    if (map->size * 100 > map->cap * MAP_HIGH_WATERMARK) {
        struct map_pair_t* old_bucket = map->bucket;
        size_t old_cap = map->cap;

        map->size = 0;
        map->cap = old_cap * 2 + 3;
        map->bucket = calloc(1, sizeof(struct map_pair_t) * map->cap);
        for (size_t idx = 0; idx < old_cap; ++idx) {
            struct map_pair_t* pair = old_bucket + idx;
            if (pair->key) {
                map_insert(map, pair->key, pair->data);
            }
        }

        free(old_bucket);
    }

    return;
}

// https://stackoverflow.com/questions/7666509/hash-function-for-string
uint hash_str(const char* str) {
    uint hash = 5381;
    for (; *str; ++str) {
        hash = ((hash << 5) + hash) + *str;  // hash * 33 + c
    }
    return hash;
}

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
