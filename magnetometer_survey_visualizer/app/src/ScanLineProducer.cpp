#include "ScanLineProducer.h"
#include "GradiometerSimulator.h"

ScanLineProducer::ScanLineProducer(int row, int grid_w, float cell_m, int n_ch,
                                    const std::vector<DipoleParams>& dipoles,
                                    uint64_t seed, LineCallback cb)
    : m_row(row), m_grid_w(grid_w), m_n_ch(n_ch), m_cell_m(cell_m),
      m_dipoles(dipoles), m_seed(seed), m_cb(std::move(cb)) {
    setAutoDelete(true);
}

void ScanLineProducer::run() {
    GradiometerSimulator sim(m_grid_w, 1, m_cell_m, m_n_ch, m_seed + static_cast<uint64_t>(m_row));
    for (const auto& dp : m_dipoles) sim.addDipole(dp);

    auto pts = sim.generate();
    for (auto& p : pts) p.y = static_cast<float>(m_row) * m_cell_m;

    if (m_cb) m_cb(std::move(pts));
}
