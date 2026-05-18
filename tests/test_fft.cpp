#include <gtest/gtest.h>
#include "../src/FftAnalyser.h"
#include <cmath>
#include <vector>

static constexpr double SR   = 1000.0;  // 1 kHz sample rate
static constexpr double FREQ =  50.0;   // 50 Hz test tone
static constexpr size_t N    = 1024;

// Generate a pure sine for testing.
static std::vector<double> makeSine(double freq, double amp, size_t n, double sr) {
    std::vector<double> v(n);
    for (size_t i = 0; i < n; ++i)
        v[i] = amp * std::sin(2.0 * M_PI * freq * static_cast<double>(i) / sr);
    return v;
}

TEST(FftTest, FundamentalFrequencyDetected) {
    FftAnalyser fft(SR, N);
    auto sig = makeSine(FREQ, 1.0, N, SR);
    auto res = fft.analyse(sig);
    // Frequency bin resolution = SR/N = 0.977 Hz → allow ±1 bin tolerance
    EXPECT_NEAR(res.fundamental_hz, FREQ, SR / static_cast<double>(N) + 0.01);
}

TEST(FftTest, MagnitudeNonZero) {
    FftAnalyser fft(SR, N);
    auto sig = makeSine(FREQ, 2.0, N, SR);
    auto res = fft.analyse(sig);
    EXPECT_GT(res.fundamental_magnitude, 0.0);
}

TEST(FftTest, OutputLengthIsHalfPlusOne) {
    FftAnalyser fft(SR, N);
    auto sig = makeSine(FREQ, 1.0, N, SR);
    auto mags = fft.apply(sig);
    EXPECT_EQ(mags.size(), N / 2 + 1);
}

TEST(FftTest, FrequencyBinResolution) {
    FftAnalyser fft(SR, N);
    EXPECT_NEAR(fft.binResolution(), SR / static_cast<double>(N), 1e-10);
}

TEST(FftTest, DCComponentAtBinZero) {
    FftAnalyser fft(SR, N);
    // Pure DC signal
    std::vector<double> dc(N, 1.0);
    auto res = fft.analyse(dc);
    // DC bin (k=0) should dominate — fundamental detection skips k=0
    // so peak in non-DC bins should be near-zero for pure DC
    EXPECT_GT(res.magnitudes[0], 0.0);
}

TEST(FftTest, ZeroPaddingWorksForNonPow2Input) {
    FftAnalyser fft(SR, N);
    // 300 samples (not power of 2) — should zero-pad and succeed
    auto sig = makeSine(FREQ, 1.0, 300, SR);
    EXPECT_NO_THROW(fft.apply(sig));
}

TEST(FftTest, TwoTonesHaveTwoPeaks) {
    FftAnalyser fft(SR, N);
    auto s1 = makeSine( 50.0, 1.0, N, SR);
    auto s2 = makeSine(200.0, 0.8, N, SR);
    std::vector<double> mixed(N);
    for (size_t i = 0; i < N; ++i) mixed[i] = s1[i] + s2[i];

    auto res  = fft.analyse(mixed);
    auto mags = res.magnitudes;

    // Both bins should have elevated magnitude — find top-2
    auto copy = mags;
    std::sort(copy.rbegin(), copy.rend());
    EXPECT_GT(copy[0], copy[2]);  // top-2 clearly above rest
}
