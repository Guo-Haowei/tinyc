#ifndef __CC_H__
#define __CC_H__
#include "stdarg.h"  // va_list
#include "stdio.h"   // printf
#include "stdlib.h"  // exit
#include "string.h"  // memset

// build target
#define DEBUG_BUILD 0
#define RELEASE_BUILD 1
#define TEST_BUILD 2

#ifndef TARGET_BUILD
#define TARGET_BUILD DEBUG_BUILD
#endif  // #ifndef TARGET_BUILD

/*
** types
*/
#define bool int
#define true 1
#define false 0

/*
** utility
*/
#define ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

#ifdef MIN
#undef MIN
#endif  // #ifdef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#ifdef MAX
#undef MAX
#endif  // #ifdef MIN
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define MAX_PATH_LEN 256

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

/*
** string_util.c
*/
struct slice_t {
    const char* start;
    size_t len;
};

struct sstream {
    char* head;
    char* tail;
    size_t cap;
};

#define make_sstream(name, capacity) \
    struct sstream name;             \
    name.head = NULL;                \
    name.tail = NULL;                \
    name.cap = 0;                    \
    sstream_reserve(&name, capacity)

void sstream_reserve(struct sstream* ss, int cap);
void sstream_clear(struct sstream* ss);

const char* sstream_get(struct sstream* ss);

void sstream_append(struct sstream* ss, const char* str);
void sstream_appendfmt(struct sstream* ss, const char* fmt, ...);
void sstream_push_back(struct sstream* ss, const char c);

const char* path_concat(const char* source, const char* header);
void shortenpath(char* inpath, size_t maxsize);

#define streq(str1, str2) (strcmp((str1), (str2)) == 0)

/*
** list.c
*/
struct list_node_t {
    struct list_node_t* prev;
    struct list_node_t* next;
    void* data;
};

struct list_t {
    struct list_node_t* front;
    struct list_node_t* back;
    int len;
};

struct list_t* list_new();
void list_delete(struct list_t* plist);
void list_clear(struct list_t* list);

#define list_empty(l) (l->len == 0)
#define list_len(l) (l->len)

void* _list_back(struct list_t* list);
void* _list_front(struct list_t* list);
void* _list_at(struct list_t* list, int idx);
#define list_back(T, l) ((T)_list_back(l))
#define list_front(T, l) ((T)_list_front(l))
#define list_at(T, l, i) ((T)_list_at(l, i))

void _list_push_front(struct list_t* list, void* data);
void _list_push_back(struct list_t* list, void* data);
#define list_push_front(l, e) _list_push_front(l, ((void*)(e)))
#define list_push_back(l, e) _list_push_back(l, ((void*)(e)))

void* _list_pop_front(struct list_t* list);
void* _list_pop_back(struct list_t* list);
#define list_pop_front(T, l) ((T)_list_pop_front(l))
#define list_pop_back(T, l) ((T)_list_pop_back(l))

/*
** map.c
*/
// This is a dummy array, with log(n) search time
struct map_pair_t {
    const char* key;
    void* data;
};

struct map_t {
    struct list_t* list;
};

struct map_t* map_new();
void map_delete(struct map_t* map);

#define map_empty(m) ((m)->list->len == 0)
#define map_len(m) ((m)->list->len)

void _map_insert(struct map_t* map, const char* key, void* data);
bool _map_try_insert(struct map_t* map, const char* key, void* data);

#define map_insert(m, k, e) _map_insert(m, k, (void*)e);
#define map_try_insert(m, k, e) _map_try_insert(m, k, (void*)e);

/// TODO: map_try_insert

struct map_pair_t* map_find(struct map_t* map, const char* key);

/*
** token.c
*/
enum {
#define TOKEN(name, symbol, kw, punct) TK_##name,
#include "token.inl"
#undef TOKEN
    TK_COUNT
};

struct Token {
    const char* path;        // source path
    const char* source;      // source file
    const char* start;       // token start
    const char* end;         // token end
    const char* macroStart;  // start of the macro expanded from, if there is one
    const char* macroEnd;    // end of the macro expanded from, if there is one
    char* raw;               // to store # or ##
    int col;                 // colomn number
    int ln;                  // line number
    int kind;                // kind of token
};

/*
** lexer.c
*/
struct list_t* lex_one(const char* path, const char* source);
struct list_t* lex(const char* path);

void init_gloabl();      // set up keyword, puncts
void shutdown_global();  // clean up keywords, puncts

/*
** preprocessor.c
*/
struct list_t* preproc(struct list_t* tokens);

/*
** filecache.c
**   - struct FileCache* fcache_get(const char* path);
**   - lex raw file if not exists, otherwise return the cached object
*/
struct Loc {
    const char* path;
    const char* source;
    const char* p;
    int ln;
    int col;
};

struct FileCache {
    char path[MAX_PATH_LEN];
    const char* source;
    /// TODO: use array instead
    struct list_t* lines;
    struct list_t* rawtks;
};

void init_fcache();
void shutdown_fcache();
struct FileCache* fcache_get(const char* path);

/*
** arena.c
*/
void init_arena();
void shutdown_arena();
void free_arena();

void* alloc(int bytes);

/*
** error.c
*/
void _panic(int line, const char* file, const char* fmt, ...);
#define panic(...) _panic(__LINE__, __FILE__, __VA_ARGS__)

enum {
    LEVEL_WARNING,
    LEVEL_ERROR,
    LEVEL_FATAL,
};

void error(const char* fmt, ...);
void error_loc(int level, const struct Loc* loc, const char* fmt, ...);
void error_tk(int level, const struct Token* tk, const char* fmt, ...);
void error_after_tk(int level, const struct Token* tk, const char* fmt, ...);
void check_should_exit();

/*
** debug.c
*/
const char* tk2str(int kind);
void dumptks(const struct list_t* tks);

void _assert_internal(int line, const char* file, const char* assertion);

// clang-format off
#if TARGET_BUILD == DEBUG_BUILD
#define cassert(cond) if (!(cond)) { _assert_internal(__LINE__, __FILE__, #cond); }
#elif TARGET_BUILD == RELEASE_BUILD
#define cassert(cond)
#elif TARGET_BUILD == TEST_BUILD
#define cassert(cond)
#endif  // #if TARGET_BUILD == DEBUG_BUILD
// clang-format on

#define debugln(...)                  \
    {                                 \
        fprintf(stderr, __VA_ARGS__); \
        putc('\n', stderr);           \
    }

/*
** global
*/
extern const char* g_prog;

#define ANSI_RED "\e[1;31m"
#define ANSI_GRN "\e[1;32m"
#define ANSI_YELLOW "\e[1;33m"
#define ANSI_BLUE "\e[1;34m"
#define ANSI_MAGENTA "\e[1;35m"
#define ANSI_CYAN "\e[1;36m"
#define ANSI_WHITE "\e[1;37m"
#define ANSI_RST "\e[0m"

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus

#endif  // __CC_H__
