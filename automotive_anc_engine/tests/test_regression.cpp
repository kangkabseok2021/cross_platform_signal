// Scenario-matrix regression tests: RPM profiles x noise levels x fault injection.
#include <gtest/gtest.h>
#include <cmath>
#include <stdexcept>
#include "anc/hybrid_anc.hpp"

using namespace anc;

static auto default_engine() {
    auto sp = make_default_secondary_path();
    auto imc_coeffs = IMCController<8>::make_lowpass(0.3f);
    return HybridANC<32, 4, 8>(0.005f, 1e-4f, sp, imc_coeffs);
}

// Simulate RPM-synchronous engine harmonics at given frequency.
static float engine_harmonic(int n, float rpm) {
    float fund_hz = rpm / 60.0f * 2.0f;  // 2nd order (firing fundamental for 4-cyl)
    float fs = 48000.0f;
    return std::sin(2.0f * 3.14159265f * fund_hz / fs * n)
         + 0.5f * std::sin(2.0f * 3.14159265f * 2 * fund_hz / fs * n)
         + 0.25f * std::sin(2.0f * 3.14159265f * 3 * fund_hz / fs * n);
}

// Low RPM (idle, 800 RPM): ANC must not crash and output stays finite.
TEST(Regression, LowRPM_OutputFinite) {
    auto engine = default_engine();
    for (int n = 0; n < 4800; ++n) {
        float noise = engine_harmonic(n, 800.0f);
        float y = engine.process(noise, noise);
        ASSERT_TRUE(std::isfinite(y)) << "Non-finite output at n=" << n;
    }
}

// High RPM (4000 RPM): ANC must not crash and output stays finite.
TEST(Regression, HighRPM_OutputFinite) {
    auto engine = default_engine();
    for (int n = 0; n < 4800; ++n) {
        float noise = engine_harmonic(n, 4000.0f);
        float y = engine.process(noise, noise);
        ASSERT_TRUE(std::isfinite(y)) << "Non-finite output at n=" << n;
    }
}

// Fault injection: mic dropout mid-run must not throw or produce NaN.
TEST(Regression, FaultInjectionMicDropoutSafe) {
    auto engine = default_engine();
    EXPECT_NO_THROW({
        for (int n = 0; n < 500; ++n) {
            float ref = (n < 200) ? engine_harmonic(n, 1500.0f) : 0.0f;
            float err = ref;
            float y = engine.process(ref, err);
            ASSERT_TRUE(std::isfinite(y));
        }
    });
}

// ANC reduces error power after convergence vs. passthrough (closed-loop simulation).
TEST(Regression, NoiseReductionAfterConvergence) {
    auto engine = default_engine();
    float rpm = 1200.0f;
    float err_anc = 0.0f, err_pass = 0.0f;
    float y_prev = 0.0f;

    for (int n = 0; n < 5000; ++n) {
        float noise = engine_harmonic(n, rpm);
        float e     = noise - y_prev;          // closed-loop error
        float y     = engine.process(noise, e);
        y_prev = y;
        if (n > 4000) {
            err_anc  += e * e;
            err_pass += noise * noise;
        }
    }
    EXPECT_LT(err_anc, err_pass);
}
