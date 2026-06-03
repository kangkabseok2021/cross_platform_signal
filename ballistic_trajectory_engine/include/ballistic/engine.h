#pragma once
#include "types.h"
#include "munitions.h"
#include <optional>
#include <vector>

namespace ballistic {

[[nodiscard]] MunitionParams extract_params(const MunitionType& m);

class BallisticEngine {
public:
    [[nodiscard]] std::optional<std::vector<TrajectoryPoint>> compute(
        double elevation_deg,
        double muzzle_velocity_ms,
        const MunitionType& munition,
        double dt_s = 0.01) const;
};

} // namespace ballistic
