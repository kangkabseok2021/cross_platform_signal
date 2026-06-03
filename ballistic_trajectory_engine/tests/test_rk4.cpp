#include <gtest/gtest.h>
#include "ballistic/rk4.h"
#include <cmath>

using namespace ballistic;

static constexpr double kPi = 3.14159265358979323846;

TEST(RK4Test, NoDragMatchesAnalyticRange) {
    // Drag-free 45° launch at 100 m/s: range = v0^2 * sin(2θ) / g = 1019.37 m
    MunitionParams no_drag{1.0, 0.0, 0.0};
    State s{0.0, 0.0,
            100.0 * std::cos(kPi / 4),
            100.0 * std::sin(kPi / 4)};
    double dt = 0.001;
    State prev = s;
    for (int i = 0; i < 500000; ++i) {
        State next = rk4_step(s, no_drag, dt);
        if (next.y < 0.0) {
            // Linear interpolation for landing x
            double alpha = s.y / (s.y - next.y);
            double x_land = s.x + alpha * (next.x - s.x);
            double analytic = 100.0 * 100.0 * std::sin(2.0 * kPi / 4) / GRAVITY;
            EXPECT_NEAR(x_land, analytic, 1.0);
            return;
        }
        prev = s;
        s = next;
    }
    FAIL() << "Trajectory did not land within time limit";
}

TEST(RK4Test, MaxAltitudeMatchesAnalytic) {
    // Drag-free vertical shot at 100 m/s: ymax = v0^2 / (2g) = 509.68 m
    MunitionParams no_drag{1.0, 0.0, 0.0};
    State s{0.0, 0.0, 0.0, 100.0};
    double dt = 0.001;
    double y_max = 0.0;
    for (int i = 0; i < 30000; ++i) {
        s = rk4_step(s, no_drag, dt);
        if (s.y > y_max) y_max = s.y;
        if (s.vy < 0.0 && s.y < 0.0) break;
    }
    double analytic = 100.0 * 100.0 / (2.0 * GRAVITY);
    EXPECT_NEAR(y_max, analytic, 0.5);
}
