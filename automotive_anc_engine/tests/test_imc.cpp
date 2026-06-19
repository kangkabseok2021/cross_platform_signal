#include <gtest/gtest.h>
#include <cmath>
#include "anc/imc.hpp"

using namespace anc;

// Zero input must produce zero output (linearity).
TEST(IMC, ZeroInputZeroOutput) {
    auto coeffs = IMCController<8>::make_lowpass(0.5f);
    IMCController<8> imc(coeffs);

    for (int i = 0; i < 50; ++i)
        EXPECT_FLOAT_EQ(imc.process(0.0f), 0.0f);
}

// IMC low-pass filter must attenuate input power (gain < 1 for Hann window).
TEST(IMC, AttenuatesBroadbandPower) {
    auto coeffs = IMCController<16>::make_lowpass(0.8f);
    IMCController<16> imc(coeffs);

    float in_power = 0.0f, out_power = 0.0f;
    for (int n = 0; n < 2000; ++n) {
        float e = std::sin(0.3f * n) + std::sin(0.7f * n + 1.1f);
        float u = imc.process(e);
        if (n > 100) { in_power += e * e; out_power += u * u; }
    }
    EXPECT_LT(out_power, in_power);
}

// Output must remain bounded for bounded input.
TEST(IMC, OutputBoundedForBoundedInput) {
    auto coeffs = IMCController<8>::make_lowpass(0.5f);
    IMCController<8> imc(coeffs);

    float max_out = 0.0f;
    for (int n = 0; n < 500; ++n) {
        float e = std::sin(0.5f * n);
        max_out = std::max(max_out, std::fabs(imc.process(e)));
    }
    EXPECT_LT(max_out, 2.0f);
}

// After reset, state is zeroed.
TEST(IMC, ResetClearsState) {
    auto coeffs = IMCController<8>::make_lowpass(0.5f);
    IMCController<8> imc(coeffs);

    for (int n = 0; n < 50; ++n) imc.process(std::sin(0.1f * n));
    imc.reset();
    EXPECT_FLOAT_EQ(imc.process(0.0f), 0.0f);
}
