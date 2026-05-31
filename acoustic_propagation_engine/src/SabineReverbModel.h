#pragma once
#include "AcousticModel.h"
#include <algorithm>
#include <cmath>
#include <string_view>

/* Sabine diffuse-field model for enclosed spaces.
 *
 * T60 = 0.161 * V / A            (Sabine, 1900)
 * R   = A / (1 - a_bar)          room constant (sabins)
 * Lp  = Lw + 10*log10(4 / R)    reverberant field SPL
 *
 * where a_bar = A / S_total is the mean absorption coefficient.
 */
class SabineReverbModel final : public IAcousticModel {
public:
    /* V: room volume (m³),  A: total absorption (m² sabin),
     * S_total: total surface area (m²) */
    explicit SabineReverbModel(float V_m3, float A_sabins, float S_total_m2)
        : V_(V_m3), A_(A_sabins), S_(S_total_m2) {}

    [[nodiscard]] float t60() const noexcept {
        return 0.161f * V_ / std::max(A_, 1e-6f);
    }

    [[nodiscard]] float computeSPL(float Lw, float /*r*/) const override {
        const float a_bar = std::min(A_ / std::max(S_, 1e-6f), 0.9999f);
        const float R     = A_ / (1.0f - a_bar);
        return Lw + 10.0f * std::log10f(4.0f / std::max(R, 1e-6f));
    }

    [[nodiscard]] std::string_view name() const noexcept override {
        return "SabineReverb";
    }

private:
    float V_;
    float A_;
    float S_;
};
