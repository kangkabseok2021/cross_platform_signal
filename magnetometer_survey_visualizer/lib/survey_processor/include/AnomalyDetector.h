#pragma once
#include "Types.h"
#include <vector>
struct AnomalyDetector {
    static std::vector<AnomalyCandidate> find(
        const float* grid, int w, int h, float threshold_nT);
};
