#include <gtest/gtest.h>
#include "EmaFilter.h"
#include <array>
#include <cmath>

// alpha=1.0 → output always equals input (no smoothing)
TEST(EmaFilterTest, AlphaOnePassthrough) {
    EmaFilter f(1.0);
    EXPECT_DOUBLE_EQ(f.process(42.0),   42.0);
    EXPECT_DOUBLE_EQ(f.process(100.0), 100.0);
    EXPECT_DOUBLE_EQ(f.process(-5.0),   -5.0);
}

// alpha ≈ 0 → very strong smoothing: output changes < 1% of step per step
TEST(EmaFilterTest, SmallAlphaStrongSmoothing) {
    EmaFilter f(0.0001);
    f.reset(0.0);
    double prev = 0.0;
    for (int i = 0; i < 10; ++i) {
        double out = f.process(100.0);
        EXPECT_LT(std::abs(out - prev), 0.01 * 100.0);
        prev = out;
    }
}

// alpha=0.5, step 0→100: analytical result reaches ≥95 within 5 steps
// S5 = 100*(1 - 0.5^5) = 100 * 0.96875 = 96.875
TEST(EmaFilterTest, StepResponseConvergence) {
    EmaFilter f(0.5);
    f.reset(0.0);
    double out = 0.0;
    for (int i = 0; i < 5; ++i)
        out = f.process(100.0);
    EXPECT_GE(out, 95.0);
}

// reset(50.0) followed by process(50.0) must return exactly 50.0
TEST(EmaFilterTest, ResetAndConstantInput) {
    EmaFilter f(0.3);
    f.process(99.0);   // dirty the internal state
    f.reset(50.0);
    EXPECT_DOUBLE_EQ(f.process(50.0), 50.0);
}

// constant input C with any alpha → steady-state = C
TEST(EmaFilterTest, ConstantInputSteadyState) {
    EmaFilter f(0.2);
    for (int i = 0; i < 100; ++i) f.process(33.3);
    EXPECT_NEAR(f.process(33.3), 33.3, 1e-6);
}

// fault spike ×10 above baseline at alpha=0.1 → smoothed never exceeds 3× warmed baseline
// warm baseline after 50 steps: s ≈ 10*(1-0.9^50) ≈ 9.948
// spike step: s = 0.1*100 + 0.9*9.948 = 18.95 < 3*9.948 = 29.84 ✓
TEST(EmaFilterTest, FaultSpikeRejection) {
    EmaFilter f(0.1);
    for (int i = 0; i < 50; ++i) f.process(10.0);
    double baseline_state = f.state();
    double spike_out = f.process(100.0);
    EXPECT_LT(spike_out, 3.0 * baseline_state);
}
