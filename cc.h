#ifndef __CC_H__
#define __CC_H__
#include <stdarg.h>  // va_list
#include <stdio.h>   // printf
#include <stdlib.h>  // exit
#include <string.h>  // memset

// TODO: add config.h
#define DEBUG

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

/// TODO: string utilities
struct string_view {
    const char* start;
    int len;
};

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

struct list_t* _list_new();
void _list_delete(struct list_t** plist);
void _list_clear(struct list_t* list);

void* _list_back(struct list_t* list);
void* _list_front(struct list_t* list);
void* _list_at(struct list_t* list, int idx);

void _list_push_front(struct list_t* list, void* data);
void _list_push_back(struct list_t* list, void* data);
void* _list_pop_front(struct list_t* list);
void* _list_pop_back(struct list_t* list);

#define list_new(l) struct list_t* l = _list_new()
#define list_delete(l) _list_delete(&l)
#define list_clear(l) _list_clear(l)
#define list_empty(l) (l->len == 0)
#define list_len(l) (l->len)

#define list_back(t, l) ((t)_list_back(l))
#define list_front(t, l) ((t)_list_front(l))
#define list_at(t, l, i) ((t)_list_at(l, i))

#define list_push_front(l, e) _list_push_front(l, ((void*)(e)))
#define list_push_back(l, e) _list_push_back(l, ((void*)(e)))
#define list_pop_front(t, l) ((t)_list_pop_front(l))
#define list_pop_back(t, l) ((t)_list_pop_back(l))

/*
** token.c
*/
enum {
    TOKEN_INVALID,
    TOKEN_SYMBOL,
    TOKEN_KEWWORD,
    TOKEN_PUNCT,
    TOKEN_INT,
    TOKEN_CHAR,
    TOKEN_STRING,
    TOKEN_COUNT,
};

struct Token {
    const char* path;        // source path
    const char* source;      // source file
    const char* start;       // token start
    const char* end;         // token end
    const char* macroStart;  // start of the macro expanded from, if there is one
    const char* macroEnd;    // end of the macro expanded from, if there is one
    const char* extra;       // to store # or ##
    int col;                 // colomn number
    int ln;                  // line number
    int kind;                // kind of token
};

bool tkeqc(const struct Token* tk, int c);
bool tkeqstr(const struct Token* tk, const char* str);

/*
** lexer.c
*/
struct list_t* lex_one(const char* path, const char* source);
struct list_t* lex(const char* path);

/*
** preprocessor.c
*/
struct list_t* preproc(struct list_t* tokens);
void init_preproc(); // set up macro table
void shutdown_preproc(); // clean up macro table

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
    char path[256];
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
void panic(const char* fmt, ...);

enum {
    LEVEL_WARNING,
    LEVEL_ERROR,
};

void error(const char* fmt, ...);
void error_loc(int level, const struct Loc* loc, const char* fmt, ...);
void error_tk(int level, const struct Token* tk, const char* fmt, ...);

/*
** debug.c
*/
#ifdef DEBUG
const char* tk2str(int kind);
void dumptks(const struct list_t* tks);

void _assert_internal(int line, const char* file, const char* assertion);

#define assert(cond) if (!(cond)) { _assert_internal(__LINE__, __FILE__, #cond); }
#else
#define assert(cond)
#endif  // #ifdef DEBUG

#define debugln(...) { fprintf(stderr, __VA_ARGS__); putc('\n', stderr); }

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
// #define RED "\x1B[31m"
// #define GRN "\x1B[32m"

#endif  // __CC_H__
