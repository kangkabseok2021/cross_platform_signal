/* demo_safe.c — correct usage with acoustic_free_grid().
 * Dumps a 5x5 SPL grid to stdout for the NumPy oracle to verify. */
#include "acoustic.h"
#include <stdio.h>
#include <math.h>

int main(void)
{
    int nx = 5, ny = 5;
    float x[] = {1.0f, 5.0f, 10.0f, 20.0f, 50.0f};
    float y[] = {1.0f, 5.0f, 10.0f, 20.0f, 50.0f};
    float Lw = 80.0f;
    float src_x = 0.0f, src_y = 0.0f;
    int   ix, iy;

    float* grid = acoustic_alloc_grid(nx, ny);
    if (!grid) { fprintf(stderr, "alloc failed\n"); return 1; }

    compute_spl_grid(Lw, src_x, src_y, x, y, nx, ny, grid);

    printf("Lw=%.1f src=(%.1f,%.1f) g_air_absorption=%.4f\n",
           Lw, src_x, src_y, g_air_absorption);
    for (iy = 0; iy < ny; ++iy) {
        for (ix = 0; ix < nx; ++ix)
            printf("%.4f ", grid[iy * nx + ix]);
        printf("\n");
    }

    acoustic_free_grid(grid);
    return 0;
}
