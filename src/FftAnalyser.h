#pragma once
#include "ISignalFilter.h"
#include <complex>
#include <vector>

struct FftResult {
    double fundamental_hz;         // frequency of peak magnitude bin
    double fundamental_magnitude;
    std::vector<double> magnitudes; // |X[k]| for k = 0..N/2
    std::vector<double> frequencies;// frequency for each bin (Hz)
};

// FFT-based analyser — Cooley-Tukey radix-2 DIT, O(N log N).
// In production: replace with FFTW3 via Conan for maximum throughput.
// Frequency bin resolution = sample_rate / N (documented per Nyquist theorem).
class FftAnalyser : public ISignalFilter {
public:
    explicit FftAnalyser(double sample_rate, size_t fft_size = 1024)
        : sr_(sample_rate), N_(fft_size) {}

    // Returns the magnitude spectrum (N/2+1 values) zero-padded if needed.
    std::vector<double> apply(const std::vector<double>& in) override;
    FftResult analyse(const std::vector<double>& in);

    const char* name() const override { return "FFT"; }
    double binResolution() const { return sr_ / static_cast<double>(N_); }

private:
    // In-place Cooley-Tukey radix-2 DIT FFT. Input length must be power of 2.
    static void fft(std::vector<std::complex<double>>& x);
    static size_t nextPow2(size_t n);

    double sr_;
    size_t N_;
};
