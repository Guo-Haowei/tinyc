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

#define uint unsigned int
#define ushort unsigned short
#define uchar unsigned char

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

#define CAST(T, a) ((T)(a))

#define MAX_PATH_LEN 256
#define MAX_SCOPE_NAME_LEN 64

#define INT_SIZE_IN_BYTE 4
#define PTR_SIZE_IN_BYTE 8

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
void sstream_reset(struct sstream* ss);
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
    uint hash;
};

struct map_t {
    struct map_pair_t* bucket;
    size_t size;
    size_t cap;
};

uint hash_str(const char* str);

struct map_t* map_new();
void map_delete(struct map_t* map);

void _map_insert(struct map_t* map, const char* key, void* data);
#define map_insert(m, k, e) _map_insert(m, k, (void*)e);

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

void init_tk_global();
void free_tk_global();

/*
** preprocessor.c
*/
struct list_t* preproc(struct list_t* tokens);

/*
** datatype.c
*/
enum {
    DT_INVALID,
    DT_INT,
    DT_CHAR,
    DT_VOID,

    DT_ARRAY,
    DT_PTR,

    DT_COUNT
};

struct DataType {
    int kind;
    int size;
    int arrLen;
    bool mut;
    struct DataType* base;
};

struct DataType* datatype_new(int kind, int size, bool mut);

const char* datatype_string(const struct DataType* dt);

void init_dt_global();
void free_dt_global();

extern struct DataType* g_char;
extern struct DataType* g_const_char;
extern struct DataType* g_int;
extern struct DataType* g_const_int;
extern struct DataType* g_void;
extern struct DataType* g_const_char_ptr;

/*
** parser.c
*/
enum {
    SYMBOL_INVALID,
    SYMBOL_VAR,
    SYMBOL_FUNC,
    SYMBOL_ENUM,
    SYMBOL_STRUCT,
    SYMBOL_COUNT
};

enum {
    SCOPE_GLOBAL,
    SCOPE_FUNCT,
    SCOPE_STMT
};

struct Node;

struct Symbol {
    int kind;  // symbol kind
    struct Token* symbol;

    // function
    struct DataType* retType;
    struct list_t* params;
    struct Node* definition;
    // variable
    struct DataType* dataType;
};

enum {
#define NODE(name, dname, stmt, expr) ND_##name,
#include "node.inl"
#undef NODE
    ND_COUNT
};

struct Node {
    int kind;

    struct Node* parent;  // stmts
    struct Token* begin;
    struct Token* end;
    struct DataType* type;

    struct list_t* stmts;
    struct Node* expr;

    /// TODO: union
    int ivalue;

    // <comp-stmt>
    // - stmts

    // <ret-stmt>
    // - expr

    // <const-expr>
    // - value
};

struct Node* parse(struct list_t* tks);

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
void free_fcache();
struct FileCache* fcache_get(const char* path);

/*
** arena.c
*/
void init_arena();
void shutdown_arena();

void free_arena();
void reset_tmp_arena();

void* allocg(size_t bytes);
void* alloct(size_t bytes);

/*
** error.c
*/
void _panic(int line, const char* file, const char* fmt, ...);
#define panic(...) _panic(__LINE__, __FILE__, __VA_ARGS__)

enum {
    LEVEL_WARNING,
    LEVEL_ERROR,
    LEVEL_FATAL
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
const char* tk2prettystr(int kind);

const char* dt2str(int kind);

const char* nd2str(int kind);

void dumptks(const struct list_t* tks);

void dumpfunc(FILE* f, const struct Symbol* func);

void dumpnode(FILE* f, struct Node* node);

void _assert_internal(int line, const char* file, const char* assertion);

// clang-format off
#if TARGET_BUILD == DEBUG_BUILD
#define cassert(cond) if (!(cond)) _assert_internal(__LINE__, __FILE__, #cond)
#elif TARGET_BUILD == RELEASE_BUILD
#define cassert(cond)
#elif TARGET_BUILD == TEST_BUILD
#define cassert(cond) if (!(cond)) printf("assertion (%s) failed on line %d, in file \"%s\"\n", #cond, __LINE__, __FILE__)
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
