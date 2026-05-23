#pragma once
#include "Types.h"
#include <cstddef>

namespace ct {

class Reconstructor {
public:
    // Reconstruct a square image from a filtered sinogram using FBP.
    // angles_rad: projection angles in radians (size must equal sino.n_angles).
    // img_size:   side length of the output image.
    // Uses std::execution::par_unseq for parallelism.
    static Image reconstruct(const Sinogram& filtered_sino,
                              const std::vector<float>& angles_rad,
                              std::size_t img_size);

    // Generate a synthetic sinogram from a phantom image (forward projection).
    // Used in tests for round-trip accuracy validation.
    static Sinogram forwardProject(const Image& phantom,
                                    const std::vector<float>& angles_rad,
                                    std::size_t n_bins);
};

} // namespace ct
