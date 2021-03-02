#include "stdlib.h"
#include "string.h"

#ifndef assert
#include "assert.h"
#endif  // #ifndef assert

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

#ifndef T
#define T int
#endif  // #ifndef T

#ifndef T_NAME
#define T_NAME(name) Int##name
#endif  // #ifndef T_NAME

#ifndef T_FUNC_NAME
#define T_FUNC_NAME(name) int_##name
#endif  // #ifndef T_FUNC_NAME
