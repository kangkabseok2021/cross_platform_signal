#include "MonitorContext.h"
#include <cmath>

MonitorContext::MonitorContext(QObject* parent)
    : QObject(parent), m_telemetry("http://localhost:8080/", this), m_elapsed_time(0.0), m_networkError(false)
{
    connect(&m_engine, &ElectrostaticEngine::voltageChanged, this, &MonitorContext::voltageChanged);
    connect(&m_engine, &ElectrostaticEngine::ionizingChanged, this, &MonitorContext::ionizingChanged);
    connect(&m_engine, &ElectrostaticEngine::dischargeEventOccurred, this, &MonitorContext::handleDischargeEvent);
    
    connect(&m_telemetry, &TelemetryClient::postFailed, this, &MonitorContext::handlePostFailed);
    connect(&m_telemetry, &TelemetryClient::eventPosted, this, &MonitorContext::handleEventPosted);

    connect(&m_ticker, &QTimer::timeout, this, &MonitorContext::onTick);
    m_ticker.start(50);
}

double MonitorContext::voltage() const { return m_engine.voltage(); }
bool MonitorContext::ionizing() const { return m_engine.ionizing(); }
QVariantList MonitorContext::decayPoints() const { return m_decayPoints; }
bool MonitorContext::networkError() const { return m_networkError; }

void MonitorContext::setInitialCharge(double q0_kv) {
    m_elapsed_time = 0.0;
    m_decayPoints.clear();
    m_engine.setInitialCharge(q0_kv);
    emit decayPointsChanged();
}

void MonitorContext::resetIonizer() {
    m_engine.setInitialCharge(m_engine.voltage());
}

void MonitorContext::onTick() {
    double dt = 0.05;
    m_engine.tick(dt);
    m_elapsed_time += dt;

    QVariantMap pt;
    pt["x"] = m_elapsed_time;
    pt["y"] = m_engine.voltage();
    m_decayPoints.append(pt);
    
    if (m_decayPoints.size() > 300) {
        m_decayPoints.removeFirst();
    }
    emit decayPointsChanged();
}

void MonitorContext::handleDischargeEvent(double voltage_kv) {
    m_engine.deployIonizer();
    m_telemetry.postDischargeEvent(voltage_kv, 0.5, m_engine.voltage());
    emit dischargeEvent(voltage_kv);
}

void MonitorContext::handlePostFailed() {
    m_networkError = true;
    emit networkErrorChanged();
}

void MonitorContext::handleEventPosted() {
    if (m_networkError) {
        m_networkError = false;
        emit networkErrorChanged();
    }
}
