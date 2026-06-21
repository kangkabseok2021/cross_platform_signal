#pragma once
#include <vector>
#include <cstdint>

struct ScanPoint {
    float x, y, z;
    int channel;
    float amplitude_nT, raw_nT;
};

struct AnomalyCandidate {
    int grid_x, grid_y;
    float depth_m, peak_nT;
    int channel;
};

struct GridBuffer {
    std::vector<float> data;
    int width = 0, height = 0, n_depths = 0;

    float& at(int depth, int row, int col) {
        return data[static_cast<std::size_t>(depth * height * width + row * width + col)];
    }
    float at(int depth, int row, int col) const {
        return data[static_cast<std::size_t>(depth * height * width + row * width + col)];
    }
};
