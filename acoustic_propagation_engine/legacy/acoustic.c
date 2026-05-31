#include "acoustic.h"
#include <math.h>
#include <stdlib.h>

/* Mutable global — non-reentrant hazard #1 */
float g_air_absorption = 0.004f;

float* acoustic_alloc_grid(int nx, int ny)
{
    /* Hazard #2: no overflow check on nx*ny before malloc */
    return (float*)malloc((size_t)nx * (size_t)ny * sizeof(float));
}

void acoustic_free_grid(float* grid)
{
    free(grid);
}

int compute_spl_grid(float Lw,
                     float src_x, float src_y,
                     float* x_coords, float* y_coords,
                     int nx, int ny,
                     float* out_grid)
{
    int ix, iy;
    float dx, dy, r, Lp;

    /* Hazard #3: no NULL check on x_coords/y_coords */
    if (out_grid == NULL) return -1;

    for (iy = 0; iy < ny; ++iy) {
        for (ix = 0; ix < nx; ++ix) {
            dx = x_coords[ix] - src_x;
            dy = y_coords[iy] - src_y;
            r  = sqrtf(dx*dx + dy*dy);

            /* Hazard #4: r=0 silently produces Inf (no guard) */
            Lp = Lw - 20.0f * log10f(r) - 11.0f - g_air_absorption * r;

            out_grid[iy * nx + ix] = Lp;
        }
    }
    return 0;
}
