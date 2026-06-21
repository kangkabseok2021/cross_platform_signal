#include "AnomalyDetector.h"

std::vector<AnomalyCandidate> AnomalyDetector::find(
        const float* grid, int w, int h, float threshold_nT) {
    std::vector<AnomalyCandidate> result;

    auto at = [&](int r, int c) -> float {
        return grid[r * w + c];
    };

    for (int r = 0; r < h; ++r) {
        for (int c = 0; c < w; ++c) {
            float v = at(r, c);
            if (v < threshold_nT) continue;

            // Local maximum in 3x3 window
            bool is_max = true;
            for (int dr = -1; dr <= 1 && is_max; ++dr) {
                for (int dc = -1; dc <= 1 && is_max; ++dc) {
                    if (dr == 0 && dc == 0) continue;
                    int nr = r + dr, nc = c + dc;
                    if (nr < 0 || nr >= h || nc < 0 || nc >= w) continue;
                    if (at(nr, nc) > v) is_max = false;
                }
            }

            if (is_max) {
                AnomalyCandidate ac{};
                ac.grid_x = c;
                ac.grid_y = r;
                ac.depth_m = 0.0f;
                ac.peak_nT = v;
                ac.channel = 0;
                result.push_back(ac);
            }
        }
    }
    return result;
}
