#pragma once

#ifdef _WIN32
  #ifdef BALLISTIC_ENGINE_EXPORTS
    #define BALLISTIC_API __declspec(dllexport)
  #else
    #define BALLISTIC_API __declspec(dllimport)
  #endif
#else
  #define BALLISTIC_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct TrajectoryPointC {
    double time_s;
    double x_m;
    double y_m;
    double speed_ms;
};

// munition_id: 0=Artillery155mm, 1=Mortar81mm, 2=APFSDS120mm
// Returns number of points written, or -1 on error (call get_last_error()).
BALLISTIC_API int compute_trajectory(
    double elevation_deg,
    double muzzle_velocity_ms,
    int munition_id,
    TrajectoryPointC* out_points,
    int max_points,
    int* out_count);

BALLISTIC_API const char* get_last_error();

#ifdef __cplusplus
}
#endif
