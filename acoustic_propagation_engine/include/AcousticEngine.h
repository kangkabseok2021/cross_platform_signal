#pragma once
#include "AcousticModel.h"
#include <memory>
#include <span>
#include <vector>

/* Runtime Strategy Pattern — swap acoustic model without recompilation. */
class AcousticEngine {
public:
    AcousticEngine();   /* default: InverseSquareLawModel */
    ~AcousticEngine() = default;

    AcousticEngine(const AcousticEngine&)            = delete;
    AcousticEngine& operator=(const AcousticEngine&) = delete;

    void setModel(std::unique_ptr<IAcousticModel> model);

    /* Returns nx*ny SPL values (dB), row-major.  RAII — no manual free. */
    [[nodiscard]] std::vector<float> computeGrid(
        float Lw,
        float src_x, float src_y,
        std::span<const float> x_coords,
        std::span<const float> y_coords) const;

    [[nodiscard]] std::string_view currentModelName() const noexcept;

private:
    std::unique_ptr<IAcousticModel> model_;
};
