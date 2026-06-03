#define BALLISTIC_ENGINE_EXPORTS
#include "ballistic/c_api.h"
#include "ballistic/engine.h"
#include <string>

static thread_local std::string g_last_error;

extern "C" {

int compute_trajectory(
    double elevation_deg,
    double muzzle_velocity_ms,
    int munition_id,
    TrajectoryPointC* out_points,
    int max_points,
    int* out_count)
{
    ballistic::MunitionType munition;
    switch (munition_id) {
        case 0: munition = ballistic::Artillery155mm{}; break;
        case 1: munition = ballistic::Mortar81mm{};    break;
        case 2: munition = ballistic::APFSDS120mm{};   break;
        default:
            g_last_error = "Invalid munition_id: must be 0, 1, or 2";
            return -1;
    }

    ballistic::BallisticEngine engine;
    auto result = engine.compute(elevation_deg, muzzle_velocity_ms, munition);
    if (!result) {
        g_last_error = "compute() returned nullopt: "
                       "check elevation (1-89 deg) and velocity (>0 m/s)";
        return -1;
    }

    const auto& vec = *result;
    int n = static_cast<int>(vec.size());
    if (n > max_points) n = max_points;
    for (int i = 0; i < n; ++i)
        out_points[i] = {vec[i].time_s, vec[i].x_m, vec[i].y_m, vec[i].speed_ms};
    *out_count = n;
    return n;
}

const char* get_last_error() {
    return g_last_error.c_str();
}

} // extern "C"
