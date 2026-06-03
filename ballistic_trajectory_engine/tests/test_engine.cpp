#include <gtest/gtest.h>
#include "ballistic/engine.h"
#include <algorithm>
#include <cmath>

using namespace ballistic;

static const BallisticEngine engine;

TEST(EngineTest, NulloptOnZeroElevation) {
    EXPECT_FALSE(engine.compute(0.0, 400.0, Artillery155mm{}).has_value());
}

TEST(EngineTest, NulloptOnNegativeVelocity) {
    EXPECT_FALSE(engine.compute(45.0, -10.0, Artillery155mm{}).has_value());
}

TEST(EngineTest, NulloptOnElevation90deg) {
    EXPECT_FALSE(engine.compute(90.0, 400.0, Artillery155mm{}).has_value());
}

TEST(EngineTest, Artillery155ReturnsNonEmptyVector) {
    auto r = engine.compute(45.0, 400.0, Artillery155mm{});
    ASSERT_TRUE(r.has_value());
    EXPECT_GT(r->size(), 0u);
    // Last point should be at ground level (y ~ 0)
    EXPECT_NEAR(r->back().y_m, 0.0, 0.1);
}

TEST(EngineTest, DragReducesRangeVsNoDrag) {
    // Drag-free analytic range at 45°, 400 m/s: v0^2/g = 16317 m
    auto r = engine.compute(45.0, 400.0, Artillery155mm{});
    ASSERT_TRUE(r.has_value());
    double range_drag = r->back().x_m;
    double range_nodrag = 400.0 * 400.0 / GRAVITY;
    EXPECT_LT(range_drag, range_nodrag);
}

TEST(EngineTest, APFSDS120FlatterThanMortar81) {
    // APFSDS: Cd=0.12 (low drag); Mortar81: Cd=0.42 (high drag)
    // At identical launch conditions APFSDS retains more velocity → greater range
    auto r_a = engine.compute(45.0, 400.0, APFSDS120mm{});
    auto r_m = engine.compute(45.0, 400.0, Mortar81mm{});
    ASSERT_TRUE(r_a.has_value());
    ASSERT_TRUE(r_m.has_value());
    double range_apfsds = r_a->back().x_m;
    double range_mortar  = r_m->back().x_m;
    EXPECT_GT(range_apfsds, range_mortar);
}
