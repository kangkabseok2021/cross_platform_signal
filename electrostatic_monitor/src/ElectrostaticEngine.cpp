#include "ElectrostaticEngine.h"
#include <cmath>

ElectrostaticEngine::ElectrostaticEngine(QObject* parent)
    : QObject(parent), m_q0_kv(0.0), m_t_decay_s(0.0), m_ionizing(false),
      m_tau_material_s(50.0), m_tau_ionizer_s(0.5), m_voltage_kv(0.0), m_event_fired(false)
{
}

double ElectrostaticEngine::computeDecay(double q0_kv, double t_s, double tau_s) noexcept {
    return q0_kv * std::exp(-t_s / tau_s);
}

double ElectrostaticEngine::voltage() const { return m_voltage_kv; }
bool ElectrostaticEngine::ionizing() const { return m_ionizing; }

void ElectrostaticEngine::tick(double dt_s) {
    m_t_decay_s += dt_s;
    double effective_tau = m_ionizing ? m_tau_ionizer_s : m_tau_material_s;
    double true_voltage = computeDecay(m_q0_kv, m_t_decay_s, effective_tau);
    
    m_adc.setTrueVoltage(true_voltage);
    m_adc.tick(dt_s);
    
    m_voltage_kv = m_adc.sample();
    emit voltageChanged();
    
    if (std::abs(m_voltage_kv) > 2.0 && !m_event_fired) {
        m_event_fired = true;
        emit dischargeEventOccurred(m_voltage_kv);
    } else if (std::abs(m_voltage_kv) <= 2.0) {
        m_event_fired = false;
    }
}

void ElectrostaticEngine::deployIonizer() {
    m_q0_kv = m_voltage_kv;
    m_t_decay_s = 0.0;
    if (!m_ionizing) {
        m_ionizing = true;
        emit ionizingChanged();
    }
}

void ElectrostaticEngine::setInitialCharge(double q0_kv) {
    m_q0_kv = q0_kv;
    m_t_decay_s = 0.0;
    if (m_ionizing) {
        m_ionizing = false;
        emit ionizingChanged();
    }
    m_voltage_kv = q0_kv;
    m_event_fired = false;
    emit voltageChanged();
}
