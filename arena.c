#include "cc.h"

#define RESERVE_SIZE_IN_BYTE (64 * 1024 * 1024)  // 64MB
#define ALIGNMENT 8
#define ROUND_UP(byte) ((byte + ALIGNMENT - 1) & (-ALIGNMENT))

struct Arena {
    char* head;
    char* tail;
    int size;
};

static struct Arena g_arena;

void init_arena() {
    shutdown_arena();

    g_arena.head = calloc(1, RESERVE_SIZE_IN_BYTE);
    if (!g_arena.head) {
        panic("arena: failed to calloc %d bytes", RESERVE_SIZE_IN_BYTE);
    }

    g_arena.tail = g_arena.head;
    g_arena.size = RESERVE_SIZE_IN_BYTE;
}

void shutdown_arena() {
    if (g_arena.head) {
        free(g_arena.head);
    }

    g_arena.head = NULL;
    g_arena.tail = NULL;
    g_arena.size = 0;
}

void free_arena() {
    g_arena.tail = g_arena.head;
    memset(g_arena.head, 0, g_arena.size);
}

void* alloc(int bytes) {
    assert(bytes);
    if (bytes == 0) {
        return NULL;
    }

    int aligned_bytes = ROUND_UP(bytes);

    if (g_arena.head + aligned_bytes >= g_arena.tail + g_arena.size) {
        panic("arena: failed to alloc %d bytes (%d after alignment)", bytes, aligned_bytes);
    }

    char* p = g_arena.tail;
    g_arena.tail += aligned_bytes;
    return p;
}
