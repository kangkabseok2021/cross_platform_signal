#include "sobel_kernel.cuh"

static constexpr int BLOCK = 16;
static constexpr int TILE  = BLOCK + 2;   // 18×18 shared-memory halo

__global__ void sobel_kernel(
    const uint8_t* __restrict__ in,
    uint8_t*       __restrict__ out,
    int width, int height)
{
    __shared__ uint8_t smem[TILE][TILE];

    const int tx = threadIdx.x;
    const int ty = threadIdx.y;
    const int gx = blockIdx.x * BLOCK + tx;   // global x
    const int gy = blockIdx.y * BLOCK + ty;   // global y

    // ── Cooperative tile load ─────────────────────────────────────────────────
    // Each thread loads its centre pixel at smem[ty+1][tx+1].
    smem[ty + 1][tx + 1] = (gx < width && gy < height)
        ? in[gy * width + gx] : 0;

    // Left column halo
    if (tx == 0)
        smem[ty + 1][0] = (gx - 1 >= 0 && gy < height)
            ? in[gy * width + (gx - 1)] : 0;
    // Right column halo
    if (tx == BLOCK - 1)
        smem[ty + 1][TILE - 1] = (gx + 1 < width && gy < height)
            ? in[gy * width + (gx + 1)] : 0;
    // Top row halo
    if (ty == 0)
        smem[0][tx + 1] = (gy - 1 >= 0 && gx < width)
            ? in[(gy - 1) * width + gx] : 0;
    // Bottom row halo
    if (ty == BLOCK - 1)
        smem[TILE - 1][tx + 1] = (gy + 1 < height && gx < width)
            ? in[(gy + 1) * width + gx] : 0;

    // Corners
    if (tx == 0 && ty == 0)
        smem[0][0] = (gx-1>=0 && gy-1>=0) ? in[(gy-1)*width+(gx-1)] : 0;
    if (tx == BLOCK-1 && ty == 0)
        smem[0][TILE-1] = (gx+1<width && gy-1>=0) ? in[(gy-1)*width+(gx+1)] : 0;
    if (tx == 0 && ty == BLOCK-1)
        smem[TILE-1][0] = (gx-1>=0 && gy+1<height) ? in[(gy+1)*width+(gx-1)] : 0;
    if (tx == BLOCK-1 && ty == BLOCK-1)
        smem[TILE-1][TILE-1] = (gx+1<width && gy+1<height) ? in[(gy+1)*width+(gx+1)] : 0;

    __syncthreads();

    // ── Out-of-bounds or border pixels → black ────────────────────────────────
    if (gx >= width || gy >= height ||
        gx == 0 || gy == 0 || gx == width - 1 || gy == height - 1) {
        if (gx < width && gy < height) out[gy * width + gx] = 0;
        return;
    }

    // ── Sobel using shared memory (no global memory reads) ────────────────────
    const int i = ty + 1, j = tx + 1;   // centre in shared memory

    int sx = -smem[i-1][j-1] + smem[i-1][j+1]
             -2*smem[i][j-1] + 2*smem[i][j+1]
             -smem[i+1][j-1] + smem[i+1][j+1];

    int sy = -smem[i-1][j-1] - 2*smem[i-1][j] - smem[i-1][j+1]
             +smem[i+1][j-1] + 2*smem[i+1][j] + smem[i+1][j+1];

    int mag = abs(sx) + abs(sy);    // L1 norm (faster than sqrt on GPU)
    out[gy * width + gx] = static_cast<uint8_t>(mag > 255 ? 255 : mag);
}
