#include "ballistic/engine.h"
#include "ballistic/rk4.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ballistic {

MunitionParams extract_params(const MunitionType& m) {
    return std::visit([](auto&& t) {
        return MunitionParams{t.mass_kg, t.cd, t.area_m2};
    }, m);
}

std::optional<std::vector<TrajectoryPoint>> BallisticEngine::compute(
    double elevation_deg,
    double muzzle_velocity_ms,
    const MunitionType& munition,
    double dt_s) const
{
    if (elevation_deg <= 0.0 || elevation_deg >= 90.0) return std::nullopt;
    if (muzzle_velocity_ms <= 0.0) return std::nullopt;

    double theta = elevation_deg * M_PI / 180.0;
    MunitionParams p = extract_params(munition);

    State s{0.0, 0.0,
            muzzle_velocity_ms * std::cos(theta),
            muzzle_velocity_ms * std::sin(theta)};

    int max_steps = static_cast<int>(MAX_FLIGHT_TIME_S / dt_s);
    std::vector<TrajectoryPoint> pts;
    pts.reserve(static_cast<size_t>(max_steps));

    double t = 0.0;
    for (int i = 0; i < max_steps; ++i) {
        pts.push_back({t, s.x, s.y, std::hypot(s.vx, s.vy)});
        State next = rk4_step(s, p, dt_s);
        if (next.y < 0.0) {
            // Linear interpolation to exact landing point
            double alpha = s.y / (s.y - next.y);
            pts.push_back({
                t + alpha * dt_s,
                s.x  + alpha * (next.x  - s.x),
                0.0,
                std::hypot(s.vx + alpha*(next.vx-s.vx), s.vy + alpha*(next.vy-s.vy))
            });
            break;
        }
        s = next;
        t += dt_s;
    }
    return pts;
}

} // namespace ballistic
