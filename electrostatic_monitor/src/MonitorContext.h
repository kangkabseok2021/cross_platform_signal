#pragma once
#include <QObject>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>
#include "ElectrostaticEngine.h"
#include "TelemetryClient.h"
#include <QtQml/qqmlregistration.h>

class MonitorContext : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(double voltage READ voltage NOTIFY voltageChanged)
    Q_PROPERTY(bool ionizing READ ionizing NOTIFY ionizingChanged)
    Q_PROPERTY(QVariantList decayPoints READ decayPoints NOTIFY decayPointsChanged)
    Q_PROPERTY(bool networkError READ networkError NOTIFY networkErrorChanged)

public:
    explicit MonitorContext(QObject* parent = nullptr);

    double voltage() const;
    bool ionizing() const;
    QVariantList decayPoints() const;
    bool networkError() const;

    Q_INVOKABLE void setInitialCharge(double q0_kv);
    Q_INVOKABLE void resetIonizer();

signals:
    void voltageChanged();
    void ionizingChanged();
    void decayPointsChanged();
    void networkErrorChanged();
    void dischargeEvent(double v);

private slots:
    void onTick();
    void handleDischargeEvent(double voltage_kv);
    void handlePostFailed();
    void handleEventPosted();

private:
    ElectrostaticEngine m_engine;
    TelemetryClient m_telemetry;
    QTimer m_ticker;
    QVariantList m_decayPoints;
    double m_elapsed_time;
    bool m_networkError;
};
