#pragma once
#include "AcousticModel.h"
#include <algorithm>
#include <cmath>
#include <future>
#include <span>
#include <thread>
#include <vector>

/* Header-only parallel grid dispatcher using std::async.
 * Splits y-rows into hardware_concurrency() strips; each strip runs
 * on its own thread.  No shared mutable state — model + Lw are read-only. */
template<AcousticModelConcept M>
[[nodiscard]] std::vector<float> computeGridAsync(
    const M&               model,
    float                  Lw,
    float                  src_x,
    float                  src_y,
    std::span<const float> x_coords,
    std::span<const float> y_coords)
{
    const auto nx = static_cast<int>(x_coords.size());
    const auto ny = static_cast<int>(y_coords.size());
    std::vector<float> result(static_cast<size_t>(nx * ny));

    const int n_threads = static_cast<int>(
        std::max(1u, std::thread::hardware_concurrency()));
    const int rows_per_thread = std::max(1, (ny + n_threads - 1) / n_threads);

    std::vector<std::future<void>> futures;
    futures.reserve(static_cast<size_t>(n_threads));

    for (int t = 0; t < n_threads; ++t) {
        const int y_start = t * rows_per_thread;
        if (y_start >= ny) break;
        const int y_end = std::min(y_start + rows_per_thread, ny);

        futures.push_back(std::async(std::launch::async,
            [&model, Lw, src_x, src_y, x_coords, y_coords,
             nx, y_start, y_end, &result]() {
                for (int iy = y_start; iy < y_end; ++iy) {
                    for (int ix = 0; ix < nx; ++ix) {
                        const float dx = x_coords[static_cast<size_t>(ix)] - src_x;
                        const float dy = y_coords[static_cast<size_t>(iy)] - src_y;
                        const float r  = std::sqrt(dx*dx + dy*dy);
                        result[static_cast<size_t>(iy * nx + ix)] =
                            model.computeSPL(Lw, r);
                    }
                }
            }));
    }
    for (auto& f : futures) f.get();
    return result;
}
