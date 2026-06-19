#pragma once
#include <array>
#include <cmath>
#include <cstddef>
#include <numeric>
#include "secondary_path.hpp"

namespace anc {

// Filtered-x LMS adaptive filter: cancels reference-correlated tonal components.
//   W  = number of adaptive filter taps
//   SP = secondary path tap count
template <std::size_t W, std::size_t SP>
class FxLMS {
public:
    FxLMS(float mu, float leakage, const SecondaryPath<SP>& sec_path)
        : mu_(mu), leakage_(leakage), sec_path_(sec_path) {
        weights_.fill(0.0f);
        ref_buf_.fill(0.0f);
        filt_ref_buf_.fill(0.0f);
    }

    // Process one sample. Returns the anti-noise output y(n).
    // ref:   reference microphone (tachometer-correlated)
    // error: error microphone (residual noise at cancellation point)
    float process(float ref, float error) noexcept {
        // Buffer reference signal
        ref_buf_[pos_] = ref;

        // Filtered reference: pass ref through secondary-path estimate
        float filt_ref = sec_path_.apply(ref);
        filt_ref_buf_[pos_] = filt_ref;

        // Anti-noise output y(n) = w^T * x
        float y = 0.0f;
        for (std::size_t i = 0; i < W; ++i)
            y += weights_[i] * ref_buf_[(pos_ + W - i) % W];

        // Leaky LMS weight update: w(n+1) = (1-leakage)*w(n) + mu * x_hat(n) * e(n)
        for (std::size_t i = 0; i < W; ++i) {
            weights_[i] = (1.0f - leakage_) * weights_[i]
                        + mu_ * filt_ref_buf_[(pos_ + W - i) % W] * error;
        }

        pos_ = (pos_ + 1) % W;
        return y;
    }

    float weights_norm_sq() const noexcept {
        float s = 0.0f;
        for (float w : weights_) s += w * w;
        return s;
    }

    const std::array<float, W>& weights() const noexcept { return weights_; }

    void reset() noexcept {
        weights_.fill(0.0f);
        ref_buf_.fill(0.0f);
        filt_ref_buf_.fill(0.0f);
        sec_path_.reset();
        pos_ = 0;
    }

    void freeze() noexcept { frozen_ = true; }
    void unfreeze() noexcept { frozen_ = false; }
    bool is_frozen() const noexcept { return frozen_; }

private:
    float mu_;
    float leakage_;
    SecondaryPath<SP> sec_path_;
    std::array<float, W> weights_{};
    std::array<float, W> ref_buf_{};
    std::array<float, W> filt_ref_buf_{};
    std::size_t pos_ = 0;
    bool frozen_ = false;
};

} // namespace anc
