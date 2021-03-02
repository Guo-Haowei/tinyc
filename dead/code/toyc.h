#ifndef __TOYC_H__
#define __TOYC_H__
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "assert.h"
#include "stdarg.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#ifndef CALLOC
#define CALLOC(size) calloc(1, size)
#endif  // #define CALLOC

#ifndef FREE
#define FREE(p) free(p)
#endif  // #ifndef FREE

#ifndef bool
#define bool int
#endif  // #ifndef bool

#ifndef true
#define true 1
#endif  // #ifndef true

#ifndef false
#define false 0
#endif  // #ifndef false

#ifndef NULL
#define NULL ((void *)0)
#endif  // #ifndef NULL

// macro functions
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define MAX(a, b) (a > b ? a : b)

#define MIN(a, b) (a < b ? a : b)

#if _DEBUG
#define DEBUG_BREAK __debugbreak()
#else
#define DEBUG_BREAK exit(-1)
#endif  //#if _DEBUG

#ifdef assert
#undef assert
#endif
#define assert(cond)                                                      \
    if (!(cond)) {                                                        \
        fprintf(stderr, "Assertion failed: %s, in file %s, on line %d\n", \
                #cond, __FILE__, __LINE__);                               \
        DEBUG_BREAK;                                                      \
    }

#define assertfmt(cond, fmt, ...)                                              \
    if (!(cond)) {                                                             \
        fprintf(stderr, "Assertion failed: " fmt ", in file %s, on line %d\n", \
                __VA_ARGS__, __FILE__, __LINE__);                              \
        DEBUG_BREAK;                                                           \
    }

// config
#define MAX_PATH_SIZE 128
#define MAX_FILES_INCLUDED 32

#define INT_SIZE_IN_BYTE 4
#define POINTER_SIZE_IN_BYTE 4

// log
void log(char const *format, ...);

//------------------------------------------------------------------------------
// file cache
//-----------------------------------------------------------------------------
typedef struct
{
    char path[MAX_PATH_SIZE];
    char *content;
} FileCache;

typedef struct
{
    FileCache const *fileCache;
    char const *p;
    int ln;
    int col;
} Loc;

// add to cache if not found
FileCache *filecache_get(char const *path);

//------------------------------------------------------------------------------
// containers
//-----------------------------------------------------------------------------
#define T int
#define T_NAME(x) Int##x
#define T_FUNC_NAME(x) int_##x
#include "containers/dict_decl.h"
#undef T
#undef T_NAME
#undef T_FUNC_NAME

void debug_print_int_dict(IntDict *const dict);

//------------------------------------------------------------------------------
// tokenizer
//-----------------------------------------------------------------------------
typedef enum {
    TK_INVALID,
    TK_ID,
    TK_KEYWORD,
    TK_PUNCT,
    TK_NUMBER,
    TK_STRING,
    TK_CHAR,
    TK_COUNT,
} eToken;

typedef struct
{
    eToken kind;
    Loc loc;
    int len;
} Token;

#define T Token *
#define T_NAME(x) Token##x
#define T_FUNC_NAME(x) token_##x
#include "containers/list_decl.h"
#undef T
#undef T_NAME
#undef T_FUNC_NAME

///< take in source and generate a list of tokens
TokenList *tokenize(char const *path);
///< take in a list of tokens and generate a list of preprocessed tokens
TokenList *preprocess(TokenList *input);

bool match_str(Token const *token, char const *str);
bool match_char(Token const *token, char c);

//-----------------------------------------------------------------------------
// data type
//-----------------------------------------------------------------------------
typedef enum eDataType eDataType;
enum eDataType {
    DT_INVALID,
    DT_INT,
    DT_VOID,
    DT_CHAR,
    DT_PTR,
    DT_ARRAY,
    DT_ENUM,
    DT_STRUCT,
    DT_COUNT,
};

typedef struct DataType DataType;

struct DataType {
    eDataType kind;
    const char *keyword;   // if struct or enum
    const DataType *base;  // valid if pointer or array
    int arraySize;         // if array
    bool isConst;
};

DataType *make_empty_type();
const DataType *parse_data_type(TokenList *tokenList);
int data_type_size(const DataType *type);
const char *data_type_to_string(const DataType *type);

extern DataType *s_intType;
extern DataType *s_intptrType;

//-----------------------------------------------------------------------------
// abstract syntax tree
//-----------------------------------------------------------------------------
typedef enum {
    ND_INVALID,
    ND_NULL_EXPR,   // example: for (;;)
    ND_FUNC_DECL,   // function decl
    ND_FUNC_IMPL,   // function impl
    ND_VAR_DECL,    // variable decl
    ND_VAR,         // variable
    ND_ASSIGN,      // =
    ND_POS,         // +
    ND_NEG,         // -
    ND_NOT,         // !
    ND_ADDRESS,     // &
    ND_DEREF,       // *
    ND_OR,          // ||
    ND_AND,         // &&
    ND_EQ,          // ==
    ND_NE,          // !=
    ND_GT,          // >
    ND_GE,          // >=
    ND_LT,          // <
    ND_LE,          // <=
    ND_ADD,         // +
    ND_SUB,         // -
    ND_MUL,         // *
    ND_DIV,         // /
    ND_REM,         // %
    ND_INC_I,       // ++i
    ND_DEC_I,       // --i
    ND_I_INC,       // i++
    ND_I_DEC,       // i--
    ND_SUBSCRIPT,   // []
    ND_NUMBER,      // integer
    ND_STRING,      // string
    ND_FUNC_CALL,   // function call
    ND_RETURN,      // return
    ND_BREAK,       // break
    ND_CONTINUE,    // continue
    ND_IF_ELSE,     // if
    ND_TERNARY,     // ?:
    ND_WHILE,       // while
    ND_FOR,         // for
    ND_DO_WHILE,    // do while
    ND_EXPR_STMT,   // expression statement
    ND_COMP_STMT,   // compound statement
    ND_TRANS_UNIT,  // translation unit
    ND_COUNT
} eNode;

typedef struct Node Node;

#define T Node *
#define T_NAME(x) Node##x
#define T_FUNC_NAME(x) node_##x
#include "containers/list_decl.h"
#undef T
#undef T_NAME
#undef T_FUNC_NAME

struct Node {
    eNode kind;
    int guid;

    /// flags
    bool binary;
    bool unary;

    const DataType *dataType;

    Node *owner;  // owner of return, break, continue

    Token const *token;
    Node *expr;

    int intVal;
    char const *strVal;

    NodeList *initializer;

    /// branch
    Node *cond;  // if, ternary, while, for
    Node *ifStmt;
    Node *elseStmt;

    Node *setup;
    NodeList *increment;

    /// var
    Node *decl;
    int offset;

    /// unary node or binary node
    Node *lhs;
    Node *rhs;

    /// func, loop, branch
    Node *body;

    /// function call
    Node *identifier;
    NodeList *argList;  // func call args...

    NodeList *objList;

    /// statements inside curly braces
    NodeList *stmtList;  // compound stmt
    int scopedMemory;
    int sizeInByte;
};

Node *parse(TokenList *tokenList);

//-----------------------------------------------------------------------------
// validation
//-----------------------------------------------------------------------------
void validate(Node *node);

//-----------------------------------------------------------------------------
// codegen.c
//-----------------------------------------------------------------------------
void codegen(const Node *node, FILE *fp);

//-----------------------------------------------------------------------------
// misc
//-----------------------------------------------------------------------------
char const *eToken_to_str(eToken kind);
char const *eNode_to_str(eNode kind);
/// NOTE: this is dangerous
char const *token_to_str(Token const *tk);
char const *find_line(Loc const *loc);

#define INVALID_ESCAPE -1
int escaped(int c);

unsigned int hash_str(const char *str, int length);
char const *va(char const *format, ...);

//-----------------------------------------------------------------------------
// debug
//-----------------------------------------------------------------------------
void debug_print_token(TokenList const *tokenList);
void debug_print_node(Node const *node);

//-----------------------------------------------------------------------------
// log
//-----------------------------------------------------------------------------
void log_error_at_loc(char const *msg, Loc const *loc, int span);
void log_error_at_token(char const *msg, Token const *token);
void log_unexpected_token(Token *const token);

//-----------------------------------------------------------------------------
// utility.c
//-----------------------------------------------------------------------------

typedef struct {
    char *start;
    char *p;
    int capacity;
} Buffer;

Buffer *make_buffer(int capacity);

void clear_buffer(Buffer *buf);

void destroy_buffer(Buffer **pBuf);

void buffer_append_char(Buffer *buf, int c);

void buffer_append_str(Buffer *buf, const char *str);

void buffer_append_fmt(Buffer *buf, const char *fmt, ...);

//-----------------------------------------------------------------------------
// setup.c
//-----------------------------------------------------------------------------
#define T DataType *
#define T_NAME(x) DataType##x
#define T_FUNC_NAME(x) datatype_##x
#include "containers/dict_decl.h"
#undef T
#undef T_NAME
#undef T_FUNC_NAME

extern DataTypeDict *s_dataTypeCache;

// set up built-in types, keyword tables, etc
void setup();

#endif  // #ifndef __TOYC_H__
