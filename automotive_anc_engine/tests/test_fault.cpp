#include <gtest/gtest.h>
#include "anc/fault_detector.hpp"

using namespace anc;

// Sustained near-zero error → MIC_DROPOUT.
TEST(FaultDetector, MicDropoutDetected) {
    FaultConfig cfg;
    cfg.dropout_threshold = 1e-3f;
    cfg.window = 16;
    FaultDetector fd(cfg);

    for (int i = 0; i < 32; ++i)
        fd.update(0.0f, 0.0f, 0.0f);

    EXPECT_EQ(fd.fault(), FaultType::MIC_DROPOUT);
    EXPECT_EQ(fd.state(), ANCState::MUTED);
}

// Saturated samples → MIC_CLIP.
TEST(FaultDetector, MicClippingDetected) {
    FaultConfig cfg;
    cfg.clip_threshold = 0.99f;
    cfg.clip_count     = 4;
    cfg.window         = 16;
    FaultDetector fd(cfg);

    for (int i = 0; i < 32; ++i)
        fd.update(0.5f, 1.0f, 0.0f);

    EXPECT_EQ(fd.fault(), FaultType::MIC_CLIP);
    EXPECT_EQ(fd.state(), ANCState::MUTED);
}

// Large weight norm → DIVERGENCE → ADAPTATION_FROZEN.
TEST(FaultDetector, DivergenceDetected) {
    FaultConfig cfg;
    cfg.divergence_norm_sq = 100.0f;
    cfg.window             = 16;
    FaultDetector fd(cfg);

    for (int i = 0; i < 32; ++i)
        fd.update(0.5f, 0.5f, 200.0f);   // norm² = 200 > threshold 100

    EXPECT_EQ(fd.fault(), FaultType::DIVERGENCE);
    EXPECT_EQ(fd.state(), ANCState::ADAPTATION_FROZEN);
}

// Normal operation → NONE / ACTIVE.
TEST(FaultDetector, NormalOperationActiveState) {
    FaultConfig cfg;
    cfg.dropout_threshold = 1e-6f;
    cfg.window            = 16;
    FaultDetector fd(cfg);

    for (int i = 0; i < 32; ++i)
        fd.update(0.1f, 0.1f * std::sin(0.1f * i), 0.0f);

    EXPECT_EQ(fd.fault(), FaultType::NONE);
    EXPECT_EQ(fd.state(), ANCState::ACTIVE);
}

// clear_fault resets to ACTIVE.
TEST(FaultDetector, ClearFaultResetsState) {
    FaultConfig cfg;
    cfg.window = 8;
    cfg.dropout_threshold = 1e-3f;
    FaultDetector fd(cfg);

    for (int i = 0; i < 16; ++i) fd.update(0.0f, 0.0f, 0.0f);
    ASSERT_NE(fd.fault(), FaultType::NONE);

    fd.clear_fault();
    EXPECT_EQ(fd.fault(), FaultType::NONE);
    EXPECT_EQ(fd.state(), ANCState::ACTIVE);
}
