#pragma once
#include "AudioConfig.h"
#include "AudioRingBuffer.h"
#include "IirFilter.h"
#include <atomic>
#include <array>
#include <thread>
#include <chrono>
#include <cstdint>

class AudioEngine {
public:
    AudioEngine()  = default;
    ~AudioEngine() { stop(); }

    void start();
    void stop();

    void setCutoff(float hz) noexcept;
    [[nodiscard]] float cutoff() const noexcept;

    // Snapshot of the last kWaveformSize processed samples for display.
    // Copied without a lock — a partial read is acceptable for a waveform view.
    [[nodiscard]] std::array<float, audio::kWaveformSize> waveformSnapshot() const noexcept;

    [[nodiscard]] audio::Diagnostics diagnostics() const noexcept;

    // Test hook: inject a synthetic overrun counter increment.
    void testInjectOverrun() noexcept {
        overruns_.fetch_add(1, std::memory_order_relaxed);
    }

private:
    void producerLoop(std::stop_token st);
    void consumerLoop(std::stop_token st);

    AudioRingBuffer<float, audio::kRingBufferSize> ring_;
    IirFilter filter_{audio::kDefaultCutoffHz, audio::kSampleRateHz};

    alignas(64) std::atomic<float>    cutoff_hz_{audio::kDefaultCutoffHz};
    alignas(64) std::atomic<uint64_t> overruns_{0};
    alignas(64) std::atomic<uint64_t> underruns_{0};
    alignas(64) std::atomic<uint64_t> samples_processed_{0};

    std::array<float, audio::kWaveformSize> waveform_{};
    std::chrono::steady_clock::time_point   start_time_{};

    std::jthread producer_thread_;
    std::jthread consumer_thread_;
};
