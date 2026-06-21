#include "GradiometerSimulator.h"
GradiometerSimulator::GradiometerSimulator(int w, int h, float cell, int ch, uint64_t seed)
    : m_w(w), m_h(h), m_cell(cell), m_channels(ch), m_seed(seed) {}
void GradiometerSimulator::addDipole(DipoleParams p) { m_dipoles.push_back(p); }
std::vector<ScanPoint> GradiometerSimulator::generate() { return {}; }
