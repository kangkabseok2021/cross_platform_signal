#include "FftAnalyser.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>

// Cooley-Tukey radix-2 DIT FFT in-place.
// Complexity: O(N log N). Input must be power-of-2 length.
void FftAnalyser::fft(std::vector<std::complex<double>>& x) {
    size_t N = x.size();
    assert((N & (N - 1)) == 0 && "FFT length must be power of 2");

    // Bit-reversal permutation
    for (size_t i = 1, j = 0; i < N; ++i) {
        size_t bit = N >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(x[i], x[j]);
    }

    // Butterfly stages
    for (size_t len = 2; len <= N; len <<= 1) {
        double ang = -2.0 * M_PI / static_cast<double>(len);
        std::complex<double> wlen(std::cos(ang), std::sin(ang));
        for (size_t i = 0; i < N; i += len) {
            std::complex<double> w(1.0, 0.0);
            for (size_t j = 0; j < len / 2; ++j) {
                auto u = x[i + j];
                auto v = x[i + j + len / 2] * w;
                x[i + j]           = u + v;
                x[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
}

size_t FftAnalyser::nextPow2(size_t n) {
    size_t p = 1;
    while (p < n) p <<= 1;
    return p;
}

FftResult FftAnalyser::analyse(const std::vector<double>& in) {
    size_t fftLen = std::max(N_, nextPow2(in.size()));

    // Zero-pad to next power-of-2
    std::vector<std::complex<double>> buf(fftLen, {0.0, 0.0});
    for (size_t i = 0; i < std::min(in.size(), fftLen); ++i)
        buf[i] = {in[i], 0.0};

    fft(buf);

    size_t half = fftLen / 2 + 1;
    FftResult res;
    res.magnitudes.resize(half);
    res.frequencies.resize(half);

    double binRes = sr_ / static_cast<double>(fftLen);
    double peak = 0.0;
    size_t peakBin = 0;

    for (size_t k = 0; k < half; ++k) {
        res.magnitudes[k]  = std::abs(buf[k]) / static_cast<double>(fftLen);
        res.frequencies[k] = static_cast<double>(k) * binRes;
        if (k > 0 && res.magnitudes[k] > peak) {
            peak    = res.magnitudes[k];
            peakBin = k;
        }
    }

    res.fundamental_hz        = res.frequencies[peakBin];
    res.fundamental_magnitude = peak;
    return res;
}

std::vector<double> FftAnalyser::apply(const std::vector<double>& in) {
    auto res = analyse(in);
    return res.magnitudes;
}
