#include "cc.h"

#define RESERVE_GLOBAL (1024 * 1024)  // 1MB
#define RESERVE_TEMP (1024 * 1024)    // 1MB
#define ALIGNMENT 8
#define ROUND_UP(byte) ((byte + ALIGNMENT - 1) & (-ALIGNMENT))

struct Arena {
    char* head;
    char* tail;
    int size;
};

static struct Arena g_global_arena;
static struct Arena g_tmp_arena;

static void init_one_arena(struct Arena* arena, size_t reserve_size) {
    cassert(arena->head == NULL);
    char* p = calloc(1, reserve_size);
    if (!p) {
        panic("arena: failed to calloc %d bytes", reserve_size);
    }

    arena->head = p;
    arena->tail = p;
    arena->size = reserve_size;
}

static void shutdown_one_arena(struct Arena* arena) {
    cassert(arena->head);

    free(arena->head);
    arena->head = NULL;
    arena->tail = NULL;
    arena->size = 0;
}

static void* alloc_from(struct Arena* arena, size_t bytes) {
    cassert(bytes);
    if (bytes == 0) {
        return NULL;
    }

    size_t aligned_bytes = ROUND_UP(bytes);

    if (arena->head + aligned_bytes >= arena->tail + arena->size) {
        panic("arena: failed to alloc %d bytes (%d after alignment)", bytes, aligned_bytes);
    }

    char* p = arena->tail;
    arena->tail += aligned_bytes;
    return p;
}

void init_arena() {
    init_one_arena(&g_global_arena, RESERVE_GLOBAL);
    init_one_arena(&g_tmp_arena, RESERVE_TEMP);
}

void shutdown_arena() {
    shutdown_one_arena(&g_global_arena);
    shutdown_one_arena(&g_tmp_arena);
}

void free_arena() {
    g_global_arena.tail = g_global_arena.head;
    memset(g_global_arena.head, 0, g_global_arena.size);
}

void reset_tmp_arena() {
    cassert(g_tmp_arena.head);
    g_tmp_arena.tail = g_tmp_arena.head;
    memset(g_tmp_arena.head, 0, g_tmp_arena.size);
}

void* allocg(size_t bytes) {
    return alloc_from(&g_global_arena, bytes);
}

void* alloct(size_t bytes) {
    return alloc_from(&g_tmp_arena, bytes);
}
