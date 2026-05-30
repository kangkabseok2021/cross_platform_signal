#pragma once
#include <QObject>
#include <QTimer>
#include <mutex>
#include "SensorSimulator.h"
#include "EmaFilter.h"
#include "TelemetryHttpServer.h"

class TelemetryModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(double engineTempRaw      READ engineTempRaw      NOTIFY dataChanged)
    Q_PROPERTY(double engineTempFiltered READ engineTempFiltered  NOTIFY dataChanged)
    Q_PROPERTY(double batteryVRaw        READ batteryVRaw         NOTIFY dataChanged)
    Q_PROPERTY(double batteryVFiltered   READ batteryVFiltered    NOTIFY dataChanged)
    Q_PROPERTY(double oilPressureRaw     READ oilPressureRaw      NOTIFY dataChanged)
    Q_PROPERTY(double oilPressureFiltered READ oilPressureFiltered NOTIFY dataChanged)
    Q_PROPERTY(double rpmRaw             READ rpmRaw              NOTIFY dataChanged)
    Q_PROPERTY(double rpmFiltered        READ rpmFiltered         NOTIFY dataChanged)
    Q_PROPERTY(bool   engineFault        READ engineFault         NOTIFY dataChanged)
    Q_PROPERTY(bool   batteryFault       READ batteryFault        NOTIFY dataChanged)

public:
    explicit TelemetryModel(QObject* parent = nullptr);

    double engineTempRaw()       const;
    double engineTempFiltered()  const;
    double batteryVRaw()         const;
    double batteryVFiltered()    const;
    double oilPressureRaw()      const;
    double oilPressureFiltered() const;
    double rpmRaw()              const;
    double rpmFiltered()         const;
    bool   engineFault()         const;
    bool   batteryFault()        const;

    Q_INVOKABLE void setAlpha(int channelId, double alpha);
    Q_INVOKABLE void resetChannel(int channelId);
    Q_INVOKABLE void injectFault(int channelId, double magnitude);

    TelemetrySnapshot telemetrySnapshot() const;

signals:
    void dataChanged();

private slots:
    void onTick();

private:
    mutable std::mutex          mutex_;
    VehicleSensorSimulator      sim_;
    EmaFilterBank<4>            filters_;
    std::array<SensorChannel, 4> channels_{};
    QTimer*                     timer_;
};
