#pragma once
#include "assert.h"
#include "stdio.h"

#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define RST "\x1B[0m"

#define TEST_BEGIN(name)                                  \
    printf("========================================");   \
    printf("========================================\n"); \
    printf("running test %s...\n", name);                 \
    int verbose = 1;                                      \
    int passed = 0;                                       \
    int total = 0;

#define EXPECT_EQ(A, B)                                             \
    {                                                               \
        int const same = (A) == (B);                                \
        passed += same;                                             \
        total += 1;                                                 \
        if (!same) {                                                \
            printf(RED "\"%s == %s\" " RST " failed on line %d.\n", \
                   #A, #B, __LINE__);                               \
        }                                                           \
    }

#define TEST_END()                                                        \
    printf("========================================");                   \
    printf("========================================\n");                 \
    if (passed == total) {                                                \
        printf(GRN "All tests passed " RST " (%d assertions)\n", passed); \
    } else {                                                              \
        printf("Assertsions: %2d | ", total);                             \
        printf(GRN " %2d passed" RST, passed);                            \
        printf(" | " RED " %2d failed\n" RST, total - passed);            \
    }                                                                     \
    return passed < total;
