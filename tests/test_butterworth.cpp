#include <gtest/gtest.h>
#include "../src/ButterworthFilter.h"
#include <cmath>
#include <numeric>
#include <vector>

static constexpr double SR = 1000.0;

static double rms(const std::vector<double>& v) {
    double s = 0;
    for (double x : v) s += x * x;
    return std::sqrt(s / v.size());
}

static std::vector<double> sineAt(double freq, size_t n) {
    std::vector<double> v(n);
    for (size_t i = 0; i < n; ++i)
        v[i] = std::sin(2.0 * M_PI * freq * static_cast<double>(i) / SR);
    return v;
}

TEST(ButterworthTest, PassbandSignalPassesThrough) {
    ButterworthFilter filt(200.0, SR);  // 200 Hz cutoff
    auto sig = sineAt(50.0, 4096);     // 50 Hz well inside passband
    auto out = filt.apply(sig);
    // Skip transient — steady-state RMS should be close to 1/√2 ≈ 0.707
    std::vector<double> steady(out.begin() + 512, out.end());
    EXPECT_NEAR(rms(steady), 1.0 / std::sqrt(2.0), 0.05);
}

TEST(ButterworthTest, StopbandSignalAttenuated) {
    ButterworthFilter filt(100.0, SR);  // 100 Hz cutoff
    auto sig = sineAt(400.0, 4096);    // 400 Hz well above cutoff
    auto out = filt.apply(sig);
    std::vector<double> steady(out.begin() + 512, out.end());
    EXPECT_LT(rms(steady), 0.1);  // > 20 dB attenuation at 4× cutoff
}

TEST(ButterworthTest, OutputLengthMatchesInput) {
    ButterworthFilter filt(200.0, SR);
    auto sig = sineAt(50.0, 512);
    auto out = filt.apply(sig);
    EXPECT_EQ(out.size(), sig.size());
}

TEST(ButterworthTest, NameIsButterworth) {
    ButterworthFilter filt(200.0, SR);
    EXPECT_STREQ(filt.name(), "Butterworth");
}

TEST(ButterworthTest, AtCutoffIs3dBDown) {
    ButterworthFilter filt(100.0, SR);
    auto sig = sineAt(100.0, 8192);
    auto out = filt.apply(sig);
    std::vector<double> steady(out.begin() + 1024, out.end());
    // At -3 dB, RMS ≈ 1/√2 × 1/√2 = 0.5
    double r = rms(steady);
    EXPECT_GT(r, 0.35);
    EXPECT_LT(r, 0.85);
}
