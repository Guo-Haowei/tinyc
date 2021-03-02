#include "stdio.h"

// int two_sum(int target, int* nums, int numCnt) {
//     for (int i = 0; i < numCnt; ++i) {
//         for (int j = i + 1; j < numCnt; ++j) {
//             if (nums[i] + nums[j] == target) {
//                 printf("%d + %d = %d", nums[i], nums[j], target);
//                 return 1;
//             }
//         }
//     }

//     printf("target %d not found", target);
//     return 0;
// }

int main() {
    int nums[] = {1, 4, 5, 7, 8, 15, 12};
    int target = 27;
    int numCnt = 7;

    for (int i = 0; i < numCnt; ++i) {
        for (int j = i + 1; j < numCnt; ++j) {
            if (nums[i] + nums[j] == target) {
                printf("%d + %d = %d", nums[i], nums[j], target);
                return 1;
            }
        }
    }

    printf("target %d not found", target);
    return 0;
}
