#include "AdcSimulator.h"
#include <random>
#include <cmath>
#include <algorithm>

AdcSimulator::AdcSimulator()
    : m_true_voltage_kv(0.0), m_drift_kv(0.0), m_drift_rate_kv_per_s(0.0),
      m_rng(std::random_device{}()), m_noise(0.0, 0.03) {}

void AdcSimulator::setTrueVoltage(double kv) noexcept {
    m_true_voltage_kv = kv;
}

void AdcSimulator::tick(double dt_s) noexcept {
    m_drift_kv += m_drift_rate_kv_per_s * dt_s;
}

double AdcSimulator::sample() const {
    return m_true_voltage_kv + m_noise(m_rng) + m_drift_kv;
}

uint16_t AdcSimulator::sampleRaw() const {
    double v = sample();
    double counts = (v + 20.0) / 40.0 * 65535.0;
    return static_cast<uint16_t>(std::clamp(counts, 0.0, 65535.0));
}

double AdcSimulator::countsToVoltage(uint16_t counts) noexcept {
    return counts / 65535.0 * 40.0 - 20.0;
}
