#pragma once
#include <cstdint>
#include <random>
#include <algorithm>

class AdcSimulator {
public:
    AdcSimulator();
    void setTrueVoltage(double kv) noexcept;
    void tick(double dt_s) noexcept;
    double sample() const;
    uint16_t sampleRaw() const;
    static double countsToVoltage(uint16_t counts) noexcept;

private:
    double m_true_voltage_kv;
    double m_drift_kv;
    double m_drift_rate_kv_per_s;
    mutable std::mt19937 m_rng;
    mutable std::normal_distribution<double> m_noise;
};
