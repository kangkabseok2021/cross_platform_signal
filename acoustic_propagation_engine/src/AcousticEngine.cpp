#include "AcousticEngine.h"
#include "InverseSquareLawModel.h"
#include <cmath>
#include <stdexcept>

AcousticEngine::AcousticEngine()
    : model_(std::make_unique<InverseSquareLawModel>()) {}

void AcousticEngine::setModel(std::unique_ptr<IAcousticModel> model) {
    if (!model) throw std::invalid_argument("AcousticEngine: null model");
    model_ = std::move(model);
}

std::vector<float> AcousticEngine::computeGrid(
    float Lw,
    float src_x, float src_y,
    std::span<const float> x_coords,
    std::span<const float> y_coords) const
{
    const auto nx = static_cast<int>(x_coords.size());
    const auto ny = static_cast<int>(y_coords.size());
    std::vector<float> grid(static_cast<size_t>(nx * ny));

    for (int iy = 0; iy < ny; ++iy) {
        for (int ix = 0; ix < nx; ++ix) {
            const float dx = x_coords[static_cast<size_t>(ix)] - src_x;
            const float dy = y_coords[static_cast<size_t>(iy)] - src_y;
            const float r  = std::sqrt(dx*dx + dy*dy);
            grid[static_cast<size_t>(iy * nx + ix)] = model_->computeSPL(Lw, r);
        }
    }
    return grid;
}

std::string_view AcousticEngine::currentModelName() const noexcept {
    return model_->name();
}
