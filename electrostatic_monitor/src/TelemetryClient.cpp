#include "TelemetryClient.h"
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QDateTime>
#include <cmath>

TelemetryClient::TelemetryClient(const QString& endpoint, QObject* parent)
    : QObject(parent), m_endpoint(endpoint), m_retry_delay_ms(2000)
{
    m_retry_timer.setSingleShot(true);
    connect(&m_retry_timer, &QTimer::timeout, this, &TelemetryClient::retryPendingEvents);
}

void TelemetryClient::postDischargeEvent(double voltage_kv, double tau_s, double q0_kv) {
    QJsonObject payload {
        {"timestamp", QDateTime::currentDateTimeUtc().toString(Qt::ISODate)},
        {"voltage_kv", voltage_kv},
        {"q0_kv", q0_kv},
        {"tau_s", tau_s},
        {"event", "DISCHARGE_THRESHOLD_EXCEEDED"},
        {"severity", std::abs(voltage_kv) > 5.0 ? "CRITICAL" : "WARNING"},
        {"energy_j", 0.5 * 1e-9 * voltage_kv * voltage_kv}
    };
    sendEvent(payload, 1);
}

void TelemetryClient::sendEvent(const QJsonObject& payload, int attempt) {
    QNetworkRequest req((QUrl(m_endpoint)));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument doc(payload);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    QNetworkReply* reply = m_nam.post(req, data);
    connect(reply, &QNetworkReply::finished, this, [this, reply, payload, attempt]() {
        handleNetworkReply(reply, payload, attempt);
    });
}

void TelemetryClient::handleNetworkReply(QNetworkReply* reply, const QJsonObject& payload, int attempt) {
    reply->deleteLater();
    if (reply->error() == QNetworkReply::NoError) {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        m_retry_delay_ms = 2000;
        emit eventPosted(statusCode);
        if (!m_pending_events.isEmpty() && !m_retry_timer.isActive()) {
            m_retry_timer.start(100);
        }
    } else {
        if (attempt < 3) {
            m_pending_events.enqueue(payload);
            m_retry_delay_ms *= 2;
            if (!m_retry_timer.isActive()) {
                m_retry_timer.start(m_retry_delay_ms);
            }
        }
        emit postFailed(reply->errorString());
    }
}

void TelemetryClient::retryPendingEvents() {
    if (!m_pending_events.isEmpty()) {
        QJsonObject payload = m_pending_events.dequeue();
        sendEvent(payload, 2);
    }
}
