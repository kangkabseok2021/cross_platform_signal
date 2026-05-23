#pragma once
#include <vector>
#include <span>
#include <cstddef>

namespace ct {

// Sinogram: rows = projection angles, cols = detector bins
struct Sinogram {
    std::vector<float> data;
    std::size_t n_angles = 0;
    std::size_t n_bins   = 0;

    std::span<const float> row(std::size_t angle_idx) const {
        return {data.data() + angle_idx * n_bins, n_bins};
    }
    std::span<float> row(std::size_t angle_idx) {
        return {data.data() + angle_idx * n_bins, n_bins};
    }
};

// Reconstructed 2-D image (img_size × img_size), 32-bit float pixels
struct Image {
    std::vector<float> data;
    std::size_t size = 0;   // square side length

    std::span<const float> row(std::size_t y) const {
        return {data.data() + y * size, size};
    }
    float& at(std::size_t x, std::size_t y)       { return data[y * size + x]; }
    float  at(std::size_t x, std::size_t y) const { return data[y * size + x]; }
};

enum class FilterType { RamLak, SheppLogan };

} // namespace ct
