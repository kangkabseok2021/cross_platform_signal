#pragma once
#include "Types.h"
#include <vector>
#include <cstddef>

namespace ct {

class FilterBank {
public:
    // Build a frequency-domain filter kernel of length n (one-sided + mirrored).
    // Ram-Lak:    H[k] = |k| / n   (linear ramp, H[0]=0)
    // Shepp-Logan: H[k] = H_RL[k] * sinc(k/n)  (ramp windowed by sinc)
    // Returns full-length real array (size n) suitable for element-wise DFT multiply.
    static std::vector<float> buildKernel(FilterType type, std::size_t n);

    // Apply filter to a single projection row in-place using DFT convolution.
    // Computes DFT of row, multiplies by kernel, inverse DFT back.
    static void applyToRow(std::span<float> row, const std::vector<float>& kernel);

    // Filter all rows of the sinogram in-place.
    static void filterSinogram(Sinogram& sino, FilterType type);
};

} // namespace ct
