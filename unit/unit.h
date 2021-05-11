#pragma once
#include "assert.h"
#include "stdio.h"

/// TODO: refactor ansi color code
#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define RST "\x1B[0m"

static int _passed;
static int _total;

#define test_begin(name)                                  \
    printf("========================================");   \
    printf("========================================\n"); \
    printf("running unit test [%s]...\n", name);

#define expect_eq(A, B)                                                      \
    {                                                                        \
        const int a = (A);                                                   \
        const int b = (B);                                                   \
        const int same = a == b;                                             \
        _passed += same;                                                     \
        _total += 1;                                                         \
        if (!same) {                                                         \
            printf(ANSI_RED "\"%s == %s\"" ANSI_RST " failed on line %d.\n", \
                   #A, #B, __LINE__);                                        \
            printf("expect: " ANSI_GRN "%d" ANSI_RST "\n", b);               \
            printf("actual: " ANSI_RED "%d" ANSI_RST "\n", a);               \
        }                                                                    \
    }

#define test_end()                                                                   \
    printf("========================================");                              \
    printf("========================================\n");                            \
    if (_passed == _total) {                                                         \
        printf(ANSI_GRN "All tests passed " ANSI_RST " (%d assertions)\n", _passed); \
    } else {                                                                         \
        printf("Assertsions: %2d | ", _total);                                       \
        printf(ANSI_GRN " %2d passed" ANSI_RST, _passed);                            \
        printf(" | " ANSI_RED " %2d failed\n" ANSI_RST, _total - _passed);           \
    }                                                                                \
    return _passed < _total;
