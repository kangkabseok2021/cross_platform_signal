#pragma once
#include <array>
#include <cstddef>

namespace anc {

// FIR model of the loudspeaker-to-error-mic acoustic transfer function.
template <std::size_t N>
class SecondaryPath {
public:
    using Coeffs = std::array<float, N>;

    explicit SecondaryPath(const Coeffs& h) : h_(h) {}

    float apply(float x) noexcept {
        buf_[pos_] = x;
        float y = 0.0f;
        for (std::size_t i = 0; i < N; ++i)
            y += h_[i] * buf_[(pos_ + N - i) % N];
        pos_ = (pos_ + 1) % N;
        return y;
    }

    void reset() noexcept { buf_.fill(0.0f); pos_ = 0; }
    const Coeffs& coeffs() const noexcept { return h_; }

private:
    Coeffs h_;
    std::array<float, N> buf_{};
    std::size_t pos_ = 0;
};

// Convenience: create a simple 4-tap delay model (unit impulse at tap 1).
inline SecondaryPath<4> make_default_secondary_path() {
    return SecondaryPath<4>({0.0f, 0.8f, 0.15f, 0.05f});
}

} // namespace anc
