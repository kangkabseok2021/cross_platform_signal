#pragma once
#include <vector>

namespace ballistic {

struct TrajectoryPoint {
    double time_s;
    double x_m;
    double y_m;
    double speed_ms;
};

struct MunitionParams {
    double mass_kg;
    double cd;
    double area_m2;
};

struct State {
    double x, y, vx, vy;
};

struct Derivative {
    double dx, dy, dvx, dvy;
};

inline constexpr double GRAVITY         = 9.80665;
inline constexpr double AIR_DENSITY     = 1.225;
inline constexpr double MAX_FLIGHT_TIME_S = 300.0;

} // namespace ballistic
