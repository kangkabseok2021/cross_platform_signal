#include <gtest/gtest.h>
#include "AudioConfig.h"
#include "AudioRingBuffer.h"
#include "IirFilter.h"
#include "AudioEngine.h"
#include <numbers>
#include <thread>
#include <chrono>
#include <algorithm>
#include <random>
#include <cmath>

// ─── IirFilter ───────────────────────────────────────────────────────────────

TEST(IirFilter, LowCutoff_AlphaNearZero) {
    // fc=20 Hz → very small α (strong low-pass, nearly DC)
    const float a = audio::computeAlpha(20.0f, 44100.0f);
    EXPECT_GT(a, 0.0f);
    EXPECT_LT(a, 0.01f);
}

TEST(IirFilter, HighCutoff_AlphaSignificant) {
    // fc=20 kHz → large α (nearly transparent)
    const float a = audio::computeAlpha(20000.0f, 44100.0f);
    EXPECT_GT(a, 0.5f);
    EXPECT_LT(a, 1.0f);
}

TEST(IirFilter, StepResponse_SettlesAtTau) {
    // At τ = Fs/(2π·fc) samples the step response must reach ≥ 63%.
    const float fc = 1000.0f;
    const float fs = 44100.0f;
    const float pi = static_cast<float>(std::numbers::pi);
    const int   tau = static_cast<int>(fs / (2.0f * pi * fc)); // ≈ 7

    IirFilter f(fc, fs);
    float y = 0.0f;
    for (int i = 0; i <= tau; ++i) {
        y = f.process(1.0f);
    }
    EXPECT_GE(y, 0.63f);
}

TEST(IirFilter, OutputBoundedByInput) {
    // A stable first-order IIR cannot amplify — |y[n]| ≤ max|x[n]|.
    IirFilter f(1000.0f, 44100.0f);
    std::mt19937 rng{42};
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    float max_x = 0.0f, max_y = 0.0f;
    for (int i = 0; i < 50000; ++i) {
        const float x = dist(rng);
        max_x = std::max(max_x, std::abs(x));
        max_y = std::max(max_y, std::abs(f.process(x)));
    }
    EXPECT_LE(max_y, max_x + 1e-4f);
}

TEST(IirFilter, SetCutoff_ClampsToRange) {
    IirFilter f;
    f.setCutoff(0.0f);   // below min → should clamp to kMinCutoffHz
    const float alpha_min = audio::computeAlpha(audio::kMinCutoffHz, audio::kSampleRateHz);
    EXPECT_NEAR(f.alpha(), alpha_min, 1e-5f);

    f.setCutoff(1e9f);   // above max → should clamp to kMaxCutoffHz
    const float alpha_max = audio::computeAlpha(audio::kMaxCutoffHz, audio::kSampleRateHz);
    EXPECT_NEAR(f.alpha(), alpha_max, 1e-5f);
}

// ─── AudioRingBuffer ─────────────────────────────────────────────────────────

TEST(AudioRingBuffer, EmptyOnInit) {
    AudioRingBuffer<float, 8> buf;
    EXPECT_EQ(buf.available(), 0u);
    float x{};
    EXPECT_FALSE(buf.pop(x));
}

TEST(AudioRingBuffer, PushPop_SingleElement) {
    AudioRingBuffer<float, 8> buf;
    EXPECT_TRUE(buf.push(3.14f));
    float x{};
    EXPECT_TRUE(buf.pop(x));
    EXPECT_FLOAT_EQ(x, 3.14f);
}

TEST(AudioRingBuffer, FifoOrder) {
    AudioRingBuffer<float, 8> buf;
    EXPECT_TRUE(buf.push(1.0f));
    EXPECT_TRUE(buf.push(2.0f));
    EXPECT_TRUE(buf.push(3.0f));
    float a{}, b{}, c{};
    EXPECT_TRUE(buf.pop(a));
    EXPECT_TRUE(buf.pop(b));
    EXPECT_TRUE(buf.pop(c));
    EXPECT_FLOAT_EQ(a, 1.0f);
    EXPECT_FLOAT_EQ(b, 2.0f);
    EXPECT_FLOAT_EQ(c, 3.0f);
}

TEST(AudioRingBuffer, FullReturnsFalse) {
    // N=4 → capacity = N-1 = 3 usable slots
    AudioRingBuffer<float, 4> buf;
    EXPECT_TRUE(buf.push(1.0f));
    EXPECT_TRUE(buf.push(2.0f));
    EXPECT_TRUE(buf.push(3.0f));
    EXPECT_FALSE(buf.push(4.0f));   // full
}

TEST(AudioRingBuffer, Available_Correct) {
    AudioRingBuffer<float, 8> buf;
    EXPECT_TRUE(buf.push(1.0f));
    EXPECT_TRUE(buf.push(2.0f));
    EXPECT_EQ(buf.available(), 2u);
    float x{};
    EXPECT_TRUE(buf.pop(x));
    EXPECT_EQ(buf.available(), 1u);
}

// ─── AudioEngine ─────────────────────────────────────────────────────────────

TEST(AudioEngine, InjectOverrun_CountedInDiagnostics) {
    AudioEngine eng;
    eng.testInjectOverrun();
    EXPECT_EQ(eng.diagnostics().overruns, 1u);
}

TEST(AudioEngine, SamplesProcessed_AfterRun) {
    AudioEngine eng;
    eng.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    EXPECT_GT(eng.diagnostics().samples_processed, 0u);
    eng.stop();
}

TEST(AudioEngine, Waveform_HasNonzeroValues) {
    AudioEngine eng;
    eng.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    const auto snap = eng.waveformSnapshot();
    const bool any_nonzero = std::any_of(snap.begin(), snap.end(),
        [](float v){ return std::abs(v) > 1e-6f; });
    EXPECT_TRUE(any_nonzero);
    eng.stop();
}
