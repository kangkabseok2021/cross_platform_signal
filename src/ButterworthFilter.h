#pragma once
#include "ISignalFilter.h"

// 4th-order Butterworth low-pass IIR filter — Direct-Form II transposed.
// Coefficients computed analytically via bilinear transform at construction.
// Complexity: O(N) per call (4 multiply-accumulate per sample per section).
class ButterworthFilter : public ISignalFilter {
public:
    // cutoff_hz: -3 dB cutoff frequency. sample_rate: Fs in Hz.
    ButterworthFilter(double cutoff_hz, double sample_rate);

    std::vector<double> apply(const std::vector<double>& in) override;
    const char* name() const override { return "Butterworth"; }

private:
    // Two 2nd-order sections (biquads) in Direct-Form II transposed.
    struct Biquad {
        double b0, b1, b2;  // numerator coefficients
        double a1, a2;      // denominator (a0 normalised to 1)
        double w1{0}, w2{0};// state variables
        double tick(double x);
    };

    Biquad sec1_, sec2_;
};
