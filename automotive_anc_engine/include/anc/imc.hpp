#pragma once
#include <array>
#include <cstddef>

namespace anc {

// IMC (Internal Model Control) feedback controller for broadband noise.
// Implemented as a fixed-coefficient FIR low-pass filter applied to the error
// signal — the simplification of Q(z) = S_hat^{-1}(z) * F_lowpass(z)
// appropriate when secondary-path inversion is pre-computed offline (MATLAB).
template <std::size_t N>
class IMCController {
public:
    using Coeffs = std::array<float, N>;

    explicit IMCController(const Coeffs& q) : q_(q) { buf_.fill(0.0f); }

    // Returns the IMC control output u(n) from error signal e(n).
    float process(float error) noexcept {
        buf_[pos_] = error;
        float u = 0.0f;
        for (std::size_t i = 0; i < N; ++i)
            u += q_[i] * buf_[(pos_ + N - i) % N];
        pos_ = (pos_ + 1) % N;
        return u;
    }

    void reset() noexcept { buf_.fill(0.0f); pos_ = 0; }

    // Build a simple Hann-windowed low-pass IMC filter (gain < 1 for stability).
    static Coeffs make_lowpass(float gain) {
        Coeffs c{};
        constexpr float pi = 3.14159265f;
        for (std::size_t i = 0; i < N; ++i) {
            float w = 0.5f * (1.0f - std::cos(2.0f * pi * i / (N - 1)));
            c[i] = gain * w / (N / 2.0f);
        }
        return c;
    }

private:
    Coeffs q_;
    std::array<float, N> buf_{};
    std::size_t pos_ = 0;
};

} // namespace anc
