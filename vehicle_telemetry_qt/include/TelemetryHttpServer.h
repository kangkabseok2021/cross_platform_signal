#pragma once
#include "SensorSimulator.h"
#include <array>
#include <cstdint>
#include <functional>
#include <memory>

struct TelemetrySnapshot {
    std::array<SensorChannel, 4> channels;
    std::array<double, 4> alphas;
};

class TelemetryHttpServer {
public:
    using GetSnapshot = std::function<TelemetrySnapshot()>;
    using SetAlpha   = std::function<void(int, double)>;

    TelemetryHttpServer(GetSnapshot get_snap, SetAlpha set_alpha);
    ~TelemetryHttpServer();

    void start(uint16_t port);
    void stop();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
