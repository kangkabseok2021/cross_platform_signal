#include "ButterworthFilter.h"
#include <cmath>

// Butterworth poles for 4th-order filter:
// Two 2nd-order sections derived via bilinear transform.
// Reference: Proakis & Manolakis, "Digital Signal Processing", 4th ed., §8.3

static void makeBiquad(double fc, double fs, double cos_theta, double sin_theta,
                       double& b0, double& b1, double& b2,
                       double& a1, double& a2) {
    // Pre-warp cutoff frequency
    double wd  = 2.0 * M_PI * fc;
    double T   = 1.0 / fs;
    double wa  = (2.0 / T) * std::tan(wd * T / 2.0);

    // Analogue pole pair: s = wa · exp(j·θ)
    double re  = wa * cos_theta;
    double im  = wa * sin_theta;

    // Bilinear transform s → z
    double c  = 2.0 / T;
    double d0 = c * c - 2.0 * re * c + re * re + im * im;

    b0 = (wa * wa) / d0;
    b1 = 2.0 * b0;
    b2 = b0;
    a1 = (-2.0 * (c * c - re * re - im * im)) / d0;
    a2 = (c * c + 2.0 * re * c + re * re + im * im) / d0;
}

ButterworthFilter::ButterworthFilter(double cutoff_hz, double sample_rate) {
    // 4th-order Butterworth: two biquad sections.
    // Poles at angles θ = π/8 and 3π/8 (for 4th-order prototype).
    makeBiquad(cutoff_hz, sample_rate,
               -std::cos(M_PI * 3.0 / 8.0), std::sin(M_PI * 3.0 / 8.0),
               sec1_.b0, sec1_.b1, sec1_.b2, sec1_.a1, sec1_.a2);
    makeBiquad(cutoff_hz, sample_rate,
               -std::cos(M_PI * 1.0 / 8.0), std::sin(M_PI * 1.0 / 8.0),
               sec2_.b0, sec2_.b1, sec2_.b2, sec2_.a1, sec2_.a2);
}

// Direct-Form II transposed tick — numerically stable, O(1).
double ButterworthFilter::Biquad::tick(double x) {
    double y = b0 * x + w1;
    w1 = b1 * x - a1 * y + w2;
    w2 = b2 * x - a2 * y;
    return y;
}

std::vector<double> ButterworthFilter::apply(const std::vector<double>& in) {
    std::vector<double> out(in.size());
    for (size_t i = 0; i < in.size(); ++i)
        out[i] = sec2_.tick(sec1_.tick(in[i]));
    return out;
}
