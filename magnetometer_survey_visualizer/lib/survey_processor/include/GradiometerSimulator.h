#pragma once
#include "Types.h"
#include <vector>
#include <cstdint>

struct DipoleParams { float x0, y0, z0, moment; };

class GradiometerSimulator {
public:
    GradiometerSimulator(int grid_w, int grid_h, float cell_m,
                         int n_channels = 8, uint64_t seed = 42);
    void addDipole(DipoleParams p);
    std::vector<ScanPoint> generate();
private:
    int m_w, m_h, m_channels;
    float m_cell;
    uint64_t m_seed;
    std::vector<DipoleParams> m_dipoles;
};
