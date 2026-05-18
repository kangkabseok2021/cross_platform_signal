#pragma once
#include "ISignalSource.h"
#include <cmath>
#include <random>

// Generates A·sin(2πft) + Gaussian noise — pure synthetic, no file I/O.
// Complexity: O(n) per call.
class SyntheticSource : public ISignalSource {
public:
    SyntheticSource(double freq_hz, double amplitude, double noise_sigma,
                    double sample_rate, unsigned seed = 42)
        : freq_(freq_hz), amp_(amplitude), sigma_(noise_sigma),
          sr_(sample_rate), rng_(seed), dist_(0.0, noise_sigma) {}

    std::vector<double> read(size_t n) override {
        std::vector<double> out(n);
        for (size_t i = 0; i < n; ++i) {
            double t = static_cast<double>(sample_++) / sr_;
            out[i] = amp_ * std::sin(2.0 * M_PI * freq_ * t) + dist_(rng_);
        }
        return out;
    }

    double sampleRate() const override { return sr_; }

private:
    double freq_, amp_, sigma_, sr_;
    size_t sample_{0};
    std::mt19937 rng_;
    std::normal_distribution<double> dist_;
};
