#ifndef TELEMETRY_SENDER_H
#define TELEMETRY_SENDER_H

#include <string>

struct TelemetryPayload {
    std::string material;
    float confidence;
    int64_t sorted_count;
};

class TelemetrySender {
private:
    std::string host_;
    int port_;
public:
    TelemetrySender(const std::string& host, int port);
    bool send(const TelemetryPayload& payload);
};

#endif // TELEMETRY_SENDER_H
