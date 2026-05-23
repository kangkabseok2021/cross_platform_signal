#include "ReconstructionController.h"
#include <cmath>
#include <numbers>

ReconstructionController::ReconstructionController(CtImageProvider* provider,
                                                    QObject* parent)
    : QObject(parent), m_provider(provider) {}

void ReconstructionController::setFilterType(const QString& t) {
    if (m_filterType != t) { m_filterType = t; emit filterTypeChanged(); }
}

void ReconstructionController::setCutoffFreq(double f) {
    if (m_cutoffFreq != f) { m_cutoffFreq = f; emit cutoffFreqChanged(); }
}

void ReconstructionController::startReconstruction() {
    if (m_isRunning) return;
    m_isRunning = true;
    emit isRunningChanged();
    m_future = QtConcurrent::run([this] { runReconstruction(); });
}

void ReconstructionController::runReconstruction() {
    constexpr std::size_t sz       = 256;
    constexpr std::size_t n_angles = 180;
    constexpr std::size_t n_bins   = sz;

    // Shepp-Logan phantom: two concentric ellipses (simplified)
    ct::Image phantom;
    phantom.size = sz;
    phantom.data.assign(sz * sz, 0.f);
    const float cx = sz / 2.f, cy = sz / 2.f;
    for (std::size_t y = 0; y < sz; ++y) {
        for (std::size_t x = 0; x < sz; ++x) {
            float dx = (static_cast<float>(x) - cx) / (sz * 0.45f);
            float dy = (static_cast<float>(y) - cy) / (sz * 0.35f);
            if (dx*dx + dy*dy <= 1.f) phantom.at(x, y) = 1.f;
            float dx2 = (static_cast<float>(x) - cx) / (sz * 0.25f);
            float dy2 = (static_cast<float>(y) - cy) / (sz * 0.18f);
            if (dx2*dx2 + dy2*dy2 <= 1.f) phantom.at(x, y) += 0.5f;
        }
    }

    std::vector<float> angles(n_angles);
    for (std::size_t i = 0; i < n_angles; ++i)
        angles[i] = static_cast<float>(i) * static_cast<float>(std::numbers::pi) / n_angles;

    ct::FilterType ftype = (m_filterType == "SheppLogan")
                           ? ct::FilterType::SheppLogan
                           : ct::FilterType::RamLak;

    ct::Sinogram sino = ct::Reconstructor::forwardProject(phantom, angles, n_bins);
    ct::FilterBank::filterSinogram(sino, ftype);
    ct::Image recon = ct::Reconstructor::reconstruct(sino, angles, sz);

    m_provider->updateImage(recon);

    m_isRunning = false;
    emit isRunningChanged();
    emit reconstructionDone();
}
