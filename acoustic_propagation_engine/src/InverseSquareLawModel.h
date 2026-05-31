#pragma once
#include "AcousticModel.h"
#include <algorithm>
#include <cmath>
#include <string_view>

/* Atmospheric absorption at 1 kHz, 20°C, 60% RH (ISO 9613-1 Table 3) */
inline constexpr float kAirAbsorption_1kHz  = 0.004f;   /* dB/m */
inline constexpr float kMinDistance         = 0.1f;     /* m — log10(0) guard */
inline constexpr float kHardPlaneCorrection = 11.0f;    /* dB — hard reflecting plane */

class InverseSquareLawModel final : public IAcousticModel {
public:
    /* Lp = Lw - 20*log10(max(r, 0.1)) - 11 - 0.004*r  (ISO 9613-1) */
    [[nodiscard]] float computeSPL(float Lw, float r) const override {
        const float r_safe = std::max(r, kMinDistance);
        return Lw
               - 20.0f * std::log10f(r_safe)
               - kHardPlaneCorrection
               - kAirAbsorption_1kHz * r_safe;
    }
    [[nodiscard]] std::string_view name() const noexcept override {
        return "InverseSquareLaw";
    }
};
