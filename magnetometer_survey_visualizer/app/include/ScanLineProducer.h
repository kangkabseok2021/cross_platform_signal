#pragma once
#include <QRunnable>
#include <QReadWriteLock>
#include <functional>
#include "Types.h"
#include "GradiometerSimulator.h"

class ScanLineProducer : public QRunnable {
public:
    using LineCallback = std::function<void(std::vector<ScanPoint>)>;

    ScanLineProducer(int row, int grid_w, float cell_m, int n_ch,
                     const std::vector<DipoleParams>& dipoles,
                     uint64_t seed, LineCallback cb);
    void run() override;

private:
    int m_row, m_grid_w, m_n_ch;
    float m_cell_m;
    std::vector<DipoleParams> m_dipoles;
    uint64_t m_seed;
    LineCallback m_cb;
};
