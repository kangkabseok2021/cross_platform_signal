#include "AudioEngine.h"
#include <random>
#include <algorithm>
#include <numbers>

#ifdef __linux__
#  include <pthread.h>
#  include <sched.h>

static void trySetRealtimePriority(std::jthread& t, int prio) noexcept {
    sched_param sp{};
    sp.sched_priority = prio;
    // Requires CAP_SYS_NICE; silently ignored in CI environments.
    pthread_setschedparam(t.native_handle(), SCHED_FIFO, &sp);
}
#else
static void trySetRealtimePriority(std::jthread&, int) noexcept {}
#endif

void AudioEngine::start() {
    start_time_    = std::chrono::steady_clock::now();
    producer_thread_ = std::jthread([this](std::stop_token st){ producerLoop(st); });
    consumer_thread_ = std::jthread([this](std::stop_token st){ consumerLoop(st); });
    trySetRealtimePriority(producer_thread_, 70);
    trySetRealtimePriority(consumer_thread_, 60);
}

void AudioEngine::stop() {
    producer_thread_.request_stop();
    consumer_thread_.request_stop();
}

void AudioEngine::setCutoff(float hz) noexcept {
    hz = std::clamp(hz, audio::kMinCutoffHz, audio::kMaxCutoffHz);
    cutoff_hz_.store(hz, std::memory_order_relaxed);
    filter_.setCutoff(hz);
}

float AudioEngine::cutoff() const noexcept {
    return cutoff_hz_.load(std::memory_order_relaxed);
}

std::array<float, audio::kWaveformSize> AudioEngine::waveformSnapshot() const noexcept {
    return waveform_;
}

audio::Diagnostics AudioEngine::diagnostics() const noexcept {
    const auto now = std::chrono::steady_clock::now();
    const auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(
                         now - start_time_).count();
    return {
        overruns_.load(std::memory_order_relaxed),
        underruns_.load(std::memory_order_relaxed),
        samples_processed_.load(std::memory_order_relaxed),
        static_cast<uint64_t>(ms < 0 ? 0 : ms)
    };
}

// ─── producer ───────────────────────────────────────────────────────────────
// Simulates a 44.1 kHz ADC using Box-Muller Gaussian noise.
// Pushes 44 samples every 1 ms tick; counts ring overruns as dropouts.
void AudioEngine::producerLoop(std::stop_token st) {
    std::mt19937_64 rng{std::random_device{}()};
    std::normal_distribution<float> dist{0.0f, 1.0f};

    constexpr int kSamplesPerTick = 44; // ≈ 44100 / 1000

    while (!st.stop_requested()) {
        for (int i = 0; i < kSamplesPerTick; ++i) {
            if (!ring_.push(dist(rng))) {
                overruns_.fetch_add(1, std::memory_order_relaxed);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

// ─── consumer ───────────────────────────────────────────────────────────────
// Drains the ring, applies IIR filter, writes to waveform snapshot buffer.
void AudioEngine::consumerLoop(std::stop_token st) {
    size_t write_pos = 0;
    while (!st.stop_requested()) {
        float sample = 0.0f;
        bool  drained = false;
        while (ring_.pop(sample)) {
            drained = true;
            const float y = filter_.process(sample);
            waveform_[write_pos % audio::kWaveformSize] = y;
            ++write_pos;
            samples_processed_.fetch_add(1, std::memory_order_relaxed);
        }
        if (!drained) {
            underruns_.fetch_add(1, std::memory_order_relaxed);
        }
        std::this_thread::sleep_for(std::chrono::microseconds(500));
    }
}
