#pragma once
#include <cstddef>
#include <cstdint>
#include <numbers>
#include <string>

namespace audio {

constexpr float  kSampleRateHz    = 44100.0f;
constexpr float  kMinCutoffHz     = 20.0f;
constexpr float  kMaxCutoffHz     = 20000.0f;
constexpr float  kDefaultCutoffHz = 1000.0f;
constexpr size_t kRingBufferSize  = 2048;   // power of 2
constexpr size_t kWaveformSize    = 512;

struct Diagnostics {
    uint64_t overruns{0};
    uint64_t underruns{0};
    uint64_t samples_processed{0};
    uint64_t uptime_ms{0};
};

struct FilterConfig {
    float   cutoff_hz{kDefaultCutoffHz};
    std::string name;
};

// Compute first-order IIR smoothing factor: α = Δt / (RC + Δt)
// where RC = 1 / (2π·fc).  0 < α < 1 for any valid fc and fs.
constexpr float computeAlpha(float fc_hz, float fs_hz) noexcept {
    const float rc = 1.0f / (2.0f * static_cast<float>(std::numbers::pi) * fc_hz);
    const float dt = 1.0f / fs_hz;
    return dt / (rc + dt);
}

} // namespace audio
