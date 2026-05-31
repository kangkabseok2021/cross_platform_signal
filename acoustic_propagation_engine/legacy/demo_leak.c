/* demo_leak.c — deliberately leaks three grids.
 * Run under: valgrind --leak-check=full ./demo_leak
 * Expected: "definitely lost: 3 allocs"
 * This is the baseline hazard the C++20 refactor eliminates. */
#include "acoustic.h"
#include <stdio.h>

int main(void)
{
    int i;
    for (i = 0; i < 3; ++i) {
        float* grid = acoustic_alloc_grid(10, 10);
        /* BUG: acoustic_free_grid(grid) never called — 3 leaks */
        (void)grid;
    }
    printf("demo_leak: exited without freeing grids (see valgrind output)\n");
    return 0;
}
