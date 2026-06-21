#include "GaussianSmoother.h"
#include <vector>
#include <cmath>

void GaussianSmoother::apply(float* grid, int w, int h) {
    // 3x3 Gaussian kernel (sigma≈0.85)
    static constexpr float K[3][3] = {
        {0.0625f, 0.125f, 0.0625f},
        {0.125f,  0.25f,  0.125f},
        {0.0625f, 0.125f, 0.0625f}
    };

    std::vector<float> tmp(static_cast<std::size_t>(w * h), 0.0f);

    for (int r = 0; r < h; ++r) {
        for (int c = 0; c < w; ++c) {
            float val = 0.0f, wsum = 0.0f;
            for (int kr = -1; kr <= 1; ++kr) {
                for (int kc = -1; kc <= 1; ++kc) {
                    int nr = r + kr, nc = c + kc;
                    if (nr < 0 || nr >= h || nc < 0 || nc >= w) continue;
                    float kw = K[kr + 1][kc + 1];
                    val += kw * grid[nr * w + nc];
                    wsum += kw;
                }
            }
            tmp[static_cast<std::size_t>(r * w + c)] = val / wsum;
        }
    }

    for (int i = 0; i < w * h; ++i)
        grid[i] = tmp[static_cast<std::size_t>(i)];
}
