#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QQueue>
#include <QJsonObject>
#include <QTimer>

class TelemetryClient : public QObject {
    Q_OBJECT
public:
    explicit TelemetryClient(const QString& endpoint, QObject* parent = nullptr);

    void postDischargeEvent(double voltage_kv, double tau_s, double q0_kv);

signals:
    void eventPosted(int statusCode);
    void postFailed(const QString& error);

private slots:
    void retryPendingEvents();
    void handleNetworkReply(QNetworkReply* reply, const QJsonObject& payload, int attempt);

private:
    void sendEvent(const QJsonObject& payload, int attempt);

    QNetworkAccessManager m_nam;
    QString m_endpoint;
    QQueue<QJsonObject> m_pending_events;
    QTimer m_retry_timer;
    int m_retry_delay_ms;
};
