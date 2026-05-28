#pragma once
#include <QObject>
#include "AdcSimulator.h"

class ElectrostaticEngine : public QObject {
    Q_OBJECT
    Q_PROPERTY(double voltage READ voltage NOTIFY voltageChanged)
    Q_PROPERTY(bool ionizing READ ionizing NOTIFY ionizingChanged)

public:
    explicit ElectrostaticEngine(QObject* parent = nullptr);

    static double computeDecay(double q0_kv, double t_s, double tau_s) noexcept;

    double voltage() const;
    bool ionizing() const;

    void tick(double dt_s);
    Q_INVOKABLE void deployIonizer();
    Q_INVOKABLE void setInitialCharge(double q0_kv);

signals:
    void voltageChanged();
    void ionizingChanged();
    void dischargeEventOccurred(double voltage_kv);

private:
    AdcSimulator m_adc;
    double m_q0_kv;
    double m_t_decay_s;
    bool m_ionizing;
    double m_tau_material_s;
    double m_tau_ionizer_s;
    double m_voltage_kv;
    bool m_event_fired;
};
