#include <gtest/gtest.h>
#include <cmath>
#include <array>
#include "anc/fxlms.hpp"
#include "anc/secondary_path.hpp"

using namespace anc;

static SecondaryPath<4> default_sp() { return make_default_secondary_path(); }

// Weight update: secondary path h[0]=0 so filtered-ref is non-zero only after >= 2 samples.
// After two steps (ref=1, error=1), at least one weight must be non-zero.
TEST(FxLMS, WeightUpdateFormula) {
    auto sp = default_sp();
    FxLMS<8, 4> fxlms(0.01f, 0.0f, sp);

    fxlms.process(1.0f, 1.0f);  // step 1: sp delay — filt_ref still 0
    fxlms.process(1.0f, 1.0f);  // step 2: sp outputs h[1]*x[0]=0.8 → weight update fires
    float norm = fxlms.weights_norm_sq();
    EXPECT_GT(norm, 0.0f);
}

// With mu=0, weights must never change.
TEST(FxLMS, ZeroMuNoAdaptation) {
    auto sp = default_sp();
    FxLMS<8, 4> fxlms(0.0f, 0.0f, sp);

    for (int i = 0; i < 100; ++i)
        fxlms.process(0.5f, 0.3f);

    EXPECT_FLOAT_EQ(fxlms.weights_norm_sq(), 0.0f);
}

// With leakage=0.01, weights should stay bounded after many steps.
TEST(FxLMS, LeakageKeepsWeightsBounded) {
    auto sp = default_sp();
    FxLMS<16, 4> fxlms(0.05f, 0.01f, sp);

    for (int n = 0; n < 5000; ++n) {
        float ref   = std::sin(0.1f * n);
        float error = ref * 0.5f;
        fxlms.process(ref, error);
    }
    EXPECT_LT(fxlms.weights_norm_sq(), 1e4f);
}

// FxLMS converges on a single sinusoid: closed-loop error power drops over time.
TEST(FxLMS, ConvergesOnSinusoid) {
    auto sp = default_sp();
    FxLMS<32, 4> fxlms(0.01f, 1e-4f, sp);

    float omega = 0.15f;
    float error_init = 0.0f, error_final = 0.0f;
    float y_out = 0.0f;

    for (int n = 0; n < 4000; ++n) {
        float ref = std::sin(omega * n);
        float e   = ref - y_out;          // closed-loop: error = noise − anti-noise
        y_out     = fxlms.process(ref, e);
        if (n < 200)  error_init  += e * e;
        if (n > 3800) error_final += e * e;
    }
    EXPECT_LT(error_final, error_init);
}

// Output is bounded by a reasonable multiple of the input energy.
TEST(FxLMS, OutputBoundedByInputEnergy) {
    auto sp = default_sp();
    FxLMS<8, 4> fxlms(0.001f, 0.001f, sp);

    float max_out = 0.0f;
    for (int n = 0; n < 500; ++n) {
        float ref = std::sin(0.2f * n);
        float y   = fxlms.process(ref, ref);
        max_out   = std::max(max_out, std::fabs(y));
    }
    EXPECT_LT(max_out, 100.0f);
}

// After reset, all state is zero.
TEST(FxLMS, ResetClearsState) {
    auto sp = default_sp();
    FxLMS<8, 4> fxlms(0.05f, 0.0f, sp);

    for (int n = 0; n < 100; ++n)
        fxlms.process(std::sin(0.1f * n), std::sin(0.1f * n));

    fxlms.reset();
    EXPECT_FLOAT_EQ(fxlms.weights_norm_sq(), 0.0f);
}
