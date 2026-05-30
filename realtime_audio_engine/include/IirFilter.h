#pragma once
#include "AudioConfig.h"
#include <atomic>
#include <algorithm>
#include <cmath>

// First-order IIR low-pass filter: y[n] = α·x[n] + (1-α)·y[n-1]
// α is stored atomically so the Qt main thread can update the cutoff
// while the DSP consumer thread calls process() — loaded with
// memory_order_relaxed because a one-sample stale read is inaudible.
class IirFilter {
public:
    explicit IirFilter(float fc_hz = audio::kDefaultCutoffHz,
                       float fs_hz = audio::kSampleRateHz) noexcept
        : fs_hz_(fs_hz), y_prev_(0.0f)
    {
        alpha_.store(audio::computeAlpha(fc_hz, fs_hz), std::memory_order_relaxed);
    }

    [[nodiscard]] float process(float x) noexcept {
        const float a = alpha_.load(std::memory_order_relaxed);
        y_prev_ = a * x + (1.0f - a) * y_prev_;
        return y_prev_;
    }

    void setCutoff(float fc_hz) noexcept {
        fc_hz = std::clamp(fc_hz, audio::kMinCutoffHz, audio::kMaxCutoffHz);
        alpha_.store(audio::computeAlpha(fc_hz, fs_hz_), std::memory_order_relaxed);
    }

    void reset() noexcept { y_prev_ = 0.0f; }

    [[nodiscard]] float alpha() const noexcept {
        return alpha_.load(std::memory_order_relaxed);
    }

private:
    float fs_hz_;
    std::atomic<float> alpha_;
    float y_prev_;
};
