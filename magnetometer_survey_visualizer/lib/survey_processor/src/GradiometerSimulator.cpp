#include "GradiometerSimulator.h"
#include <random>
#include <cmath>

static constexpr float MU0_OVER_4PI = 100.0f;  // nT·m³/(Am²) = 1e-7 T·m³/(Am²) × 1e9 nT/T
static constexpr float B0 = 48000.0f;

static float dipole_Bz(float dx, float dy, float dz, float m) {
    float r2 = dx*dx + dy*dy + dz*dz;
    if (r2 < 1e-9f) return 0.0f;
    float r5 = std::pow(r2, 2.5f);
    return MU0_OVER_4PI * m * (2.0f*dz*dz - dx*dx - dy*dy) / r5;
}

GradiometerSimulator::GradiometerSimulator(int w, int h, float cell, int ch, uint64_t seed)
    : m_w(w), m_h(h), m_cell(cell), m_channels(ch), m_seed(seed) {}

void GradiometerSimulator::addDipole(DipoleParams p) { m_dipoles.push_back(p); }

std::vector<ScanPoint> GradiometerSimulator::generate() {
    std::mt19937_64 rng(m_seed);
    std::normal_distribution<float> noise_dist(0.0f, 2.0f);
    std::uniform_real_distribution<float> heading_dist(-0.5f, 0.5f);

    std::vector<ScanPoint> points;
    points.reserve(static_cast<std::size_t>(m_w * m_h * m_channels));

    float t_min = 0.0f;
    const float dt = 1.0f / (100.0f * 60.0f);

    for (int row = 0; row < m_h; ++row) {
        float h_err = heading_dist(rng);
        for (int ch = 0; ch < m_channels; ++ch) {
            float ch_offset = static_cast<float>(ch - m_channels / 2) * 5.0f;
            for (int ci = 0; ci < m_w; ++ci) {
                int col = (row % 2 == 0) ? ci : (m_w - 1 - ci);
                float px = static_cast<float>(col) * m_cell;
                float py = static_cast<float>(row) * m_cell;

                float raw = B0 + ch_offset + 0.5f * t_min;
                if (ci == 0 && row > 0) raw += h_err;

                for (const auto& dp : m_dipoles)
                    raw += dipole_Bz(px - dp.x0, py - dp.y0, dp.z0, dp.moment);

                raw += noise_dist(rng);

                ScanPoint sp{};
                sp.x = px; sp.y = py; sp.z = 0.5f;
                sp.channel = ch;
                sp.raw_nT = raw;
                sp.amplitude_nT = raw - B0 - ch_offset;
                points.push_back(sp);

                t_min += dt;
            }
        }
    }
    return points;
}
