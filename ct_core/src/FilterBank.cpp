#include "FilterBank.h"
#include <cmath>
#include <complex>
#include <vector>
#include <numbers>

namespace ct {

// Minimal DFT / IDFT — O(n²) adequate for small projections (n ≤ 1024)
static std::vector<std::complex<float>> dft(std::span<const float> x) {
    const std::size_t n = x.size();
    std::vector<std::complex<float>> X(n);
    for (std::size_t k = 0; k < n; ++k) {
        for (std::size_t j = 0; j < n; ++j) {
            float angle = -2.f * static_cast<float>(std::numbers::pi) * static_cast<float>(k) * static_cast<float>(j) / static_cast<float>(n);
            X[k] += x[j] * std::complex<float>{std::cos(angle), std::sin(angle)};
        }
    }
    return X;
}

static void idft(std::vector<std::complex<float>>& X, std::span<float> out) {
    const std::size_t n = X.size();
    for (std::size_t j = 0; j < n; ++j) {
        std::complex<float> sum{};
        for (std::size_t k = 0; k < n; ++k) {
            float angle = 2.f * static_cast<float>(std::numbers::pi) * static_cast<float>(k) * static_cast<float>(j) / static_cast<float>(n);
            sum += X[k] * std::complex<float>{std::cos(angle), std::sin(angle)};
        }
        out[j] = sum.real() / static_cast<float>(n);
    }
}

std::vector<float> FilterBank::buildKernel(FilterType type, std::size_t n) {
    std::vector<float> H(n, 0.f);
    const float fn = static_cast<float>(n);
    for (std::size_t k = 0; k <= n / 2; ++k) {
        float h = static_cast<float>(k) / fn;   // Ram-Lak ramp
        if (type == FilterType::SheppLogan && k > 0) {
            float x = static_cast<float>(std::numbers::pi) * k / fn;
            h *= std::sin(x) / x;               // sinc window
        }
        H[k] = h;
        if (k > 0 && k < n - k)
            H[n - k] = h;  // mirror for negative frequencies
    }
    return H;
}

void FilterBank::applyToRow(std::span<float> row, const std::vector<float>& kernel) {
    auto X = dft(row);
    for (std::size_t k = 0; k < X.size(); ++k)
        X[k] *= kernel[k];
    idft(X, row);
}

void FilterBank::filterSinogram(Sinogram& sino, FilterType type) {
    auto kernel = buildKernel(type, sino.n_bins);
    for (std::size_t i = 0; i < sino.n_angles; ++i)
        applyToRow(sino.row(i), kernel);
}

} // namespace ct
