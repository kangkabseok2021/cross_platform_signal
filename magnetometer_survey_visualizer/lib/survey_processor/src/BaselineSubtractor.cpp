#include "BaselineSubtractor.h"

void BaselineSubtractor::subtract(float* grid, int w, int h) {
    for (int r = 0; r < h; ++r) {
        float mean = 0.0f;
        for (int c = 0; c < w; ++c) mean += grid[r * w + c];
        mean /= static_cast<float>(w);
        for (int c = 0; c < w; ++c) grid[r * w + c] -= mean;
    }
}
