#include "Reconstructor.h"
#include "FilterBank.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <numbers>
// Parallel STL — requires libc++ + TBB on Linux; falls back to seq on macOS
#if __has_include(<execution>) && defined(__linux__)
  #include <execution>
  #define CT_PAR_UNSEQ std::execution::par_unseq,
#else
  #define CT_PAR_UNSEQ
#endif

namespace ct {

Image Reconstructor::reconstruct(const Sinogram& sino,
                                   const std::vector<float>& angles_rad,
                                   std::size_t img_size) {
    Image img;
    img.size = img_size;
    img.data.assign(img_size * img_size, 0.f);

    const float centre = static_cast<float>(sino.n_bins) / 2.f;
    const float img_centre = static_cast<float>(img_size) / 2.f;

    // Pixel index vector for par_unseq loop
    std::vector<std::size_t> pixel_ids(img_size * img_size);
    std::iota(pixel_ids.begin(), pixel_ids.end(), 0u);

    std::for_each(CT_PAR_UNSEQ
                  pixel_ids.begin(), pixel_ids.end(),
                  [&](std::size_t idx) {
        const std::size_t px = idx % img_size;
        const std::size_t py = idx / img_size;
        const float x = static_cast<float>(px) - img_centre;
        const float y = static_cast<float>(py) - img_centre;

        float val = 0.f;
        for (std::size_t i = 0; i < angles_rad.size(); ++i) {
            const float cos_t = std::cos(angles_rad[i]);
            const float sin_t = std::sin(angles_rad[i]);
            // t = projection of (x,y) onto detector axis
            const float t = x * cos_t + y * sin_t + centre;
            // Bilinear interpolation on the filtered projection row
            const auto row = sino.row(i);
            const int t0 = static_cast<int>(t);
            const int t1 = t0 + 1;
            if (t0 < 0 || t1 >= static_cast<int>(sino.n_bins)) continue;
            const float frac = t - static_cast<float>(t0);
            val += row[t0] * (1.f - frac) + row[t1] * frac;
        }
        img.data[idx] = val * static_cast<float>(std::numbers::pi) /
                        static_cast<float>(angles_rad.size());
    });

    return img;
}

Sinogram Reconstructor::forwardProject(const Image& phantom,
                                        const std::vector<float>& angles_rad,
                                        std::size_t n_bins) {
    Sinogram sino;
    sino.n_angles = angles_rad.size();
    sino.n_bins   = n_bins;
    sino.data.assign(sino.n_angles * n_bins, 0.f);

    const float centre     = static_cast<float>(n_bins) / 2.f;
    const float img_centre = static_cast<float>(phantom.size) / 2.f;

    for (std::size_t i = 0; i < angles_rad.size(); ++i) {
        const float cos_t = std::cos(angles_rad[i]);
        const float sin_t = std::sin(angles_rad[i]);
        auto row = sino.row(i);
        for (std::size_t b = 0; b < n_bins; ++b) {
            const float t = static_cast<float>(b) - centre;
            // Accumulate along the ray perpendicular to angle
            float sum = 0.f;
            for (std::size_t s = 0; s < phantom.size; ++s) {
                const float u = static_cast<float>(s) - img_centre;
                const float px_f = t * cos_t - u * sin_t + img_centre;
                const float py_f = t * sin_t + u * cos_t + img_centre;
                const int px = static_cast<int>(px_f);
                const int py = static_cast<int>(py_f);
                if (px >= 0 && py >= 0 &&
                    static_cast<std::size_t>(px) < phantom.size &&
                    static_cast<std::size_t>(py) < phantom.size) {
                    sum += phantom.at(px, py);
                }
            }
            row[b] = sum;
        }
    }
    return sino;
}

} // namespace ct
