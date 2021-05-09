#ifndef __CC_H__
#define __CC_H__
#include <stdarg.h>  // va_list
#include <stdio.h>   // printf
#include <stdlib.h>  // exit
#include <string.h>  // memset

// TODO: add config.h
#define DEBUG

/*
** list.c
*/
struct _list_node_t {
    struct _list_node_t* prev;
    struct _list_node_t* next;
    void* data;
};

struct _list_t {
    struct _list_node_t* front;
    struct _list_node_t* back;
    int len;
};

struct _list_t* _list_new();
void _list_delete(struct _list_t** plist);
void _list_clear(struct _list_t* list);

void* _list_back(struct _list_t* list);
void* _list_front(struct _list_t* list);

void _list_push_front(struct _list_t* list, void* data);
void _list_push_back(struct _list_t* list, void* data);
void* _list_pop_front(struct _list_t* list);
void* _list_pop_back(struct _list_t* list);

#define list_new(t, l) struct _list_t* l = _list_new()
#define list_delete(t, l) _list_delete(&l)

#define list_back(t, l) ((t)_list_back(l))
#define list_front(t, l) ((t)_list_front(l))

#define list_push_front(t, l, e) _list_push_front(l, ((void*)(e)))
#define list_push_back(t, l, e) _list_push_back(l, ((void*)(e)))
#define list_pop_front(t, l) ((t)_list_pop_front(l))
#define list_pop_back(t, l) ((t)_list_pop_back(l))

/*
** lexer.c
*/
enum TokenKind {
    TOKEN_INVALID,
    TOKEN_SYMBOL,
    TOKEN_PUNCT,
    TOKEN_INT,
    TOKEN_CHAR,
    TOKEN_STRING,
    TOKEN_COUNT,
};

struct Token {
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

struct _list_t* lex(const char* file);

/*
** arena.c
*/
void arena_init();
void arena_shutdown();
void arena_free_all();

void* alloc(int bytes);

/*
** error.c
*/
void panic(const char* fmt, ...);

void error(const char* fmt, ...);

/*
** debug.c
*/
#ifdef DEBUG
const char* tk2str(int kind);

void _assert_internal(int line, const char* file, const char* assertion);

#define assert(cond) if (!(cond)) { _assert_internal(__LINE__, __FILE__, #cond); }
#else
#define assert(cond)
#endif  // #ifdef DEBUG

#endif  // __CC_H__
