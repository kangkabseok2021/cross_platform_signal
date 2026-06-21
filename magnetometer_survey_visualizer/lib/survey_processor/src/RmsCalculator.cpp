#include "RmsCalculator.h"
#include <cmath>

float RmsCalculator::compute(const float* grid, int w, int h) {
    int n = w * h;
    if (n == 0) return 0.0f;
    float sum = 0.0f;
    for (int i = 0; i < n; ++i) sum += grid[i] * grid[i];
    return std::sqrt(sum / static_cast<float>(n));
}
