/* acoustic.h — legacy C99 point-source SPL engine
 *
 * WARNING: This API has known hazards documented in KNOWN_HAZARDS.md.
 * It is preserved as the baseline the C++20 refactor must improve upon.
 *
 * Formula: Lp = Lw - 20*log10(r) - 11 - g_air_absorption*r   (ISO 9613-1)
 */
#ifndef ACOUSTIC_H
#define ACOUSTIC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Global state — makes the module non-reentrant.  Two concurrent callers
 * with different frequencies can corrupt each other's absorption coefficient. */
extern float g_air_absorption; /* dB/m, default 0.004 at 1 kHz, 20°C, 60% RH */

/* Allocate a flat nx*ny grid.  Caller MUST call acoustic_free_grid(). */
float* acoustic_alloc_grid(int nx, int ny);

/* Free a grid returned by acoustic_alloc_grid. */
void acoustic_free_grid(float* grid);

/* Fill out_grid[ny*nx] with SPL values (dB).
 * Returns 0 on success, -1 if out_grid is NULL.
 * No bounds check on x_coords/y_coords arrays — buffer overrun if nx/ny wrong. */
int compute_spl_grid(float Lw,
                     float src_x, float src_y,
                     float* x_coords, float* y_coords,
                     int nx, int ny,
                     float* out_grid);

#ifdef __cplusplus
}
#endif
#endif /* ACOUSTIC_H */
