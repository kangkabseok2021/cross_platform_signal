#include "SensorSimulator.h"
#include "EmaFilter.h"
#include "TelemetryHttpServer.h"
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

static std::atomic<bool> g_running{true};
static void sig_handler(int) { g_running = false; }

int main(int argc, char** argv) {
    uint16_t port = 8080;
    for (int i = 1; i < argc - 1; ++i) {
        if (std::string(argv[i]) == "--port")
            port = static_cast<uint16_t>(std::atoi(argv[i + 1]));
    }

    std::signal(SIGINT,  sig_handler);
    std::signal(SIGTERM, sig_handler);

    VehicleSensorSimulator      sim;
    EmaFilterBank<4>            filters(0.1);
    std::array<SensorChannel, 4> state{};
    std::mutex                   state_mutex;

    std::thread update_thread([&] {
        while (g_running) {
            sim.update();
            auto raw = sim.rawValues();
            std::array<double, 4> filtered{};
            filters.processAll(raw, filtered);
            {
                std::lock_guard<std::mutex> lk(state_mutex);
                const auto& ch = sim.channels();
                for (std::size_t i = 0; i < 4; ++i) {
                    state[i]          = ch[i];
                    state[i].filtered = filtered[i];
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    TelemetryHttpServer server(
        [&]() -> TelemetrySnapshot {
            std::lock_guard<std::mutex> lk(state_mutex);
            return {state, {filters.alpha(0), filters.alpha(1), filters.alpha(2), filters.alpha(3)}};
        },
        [&](int ch, double a) {
            filters.setAlpha(static_cast<std::size_t>(ch), a);
        }
    );
    server.start(port);
    std::cout << "vehicle_telemetry_headless running on :" << port << "\n" << std::flush;

    while (g_running)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    server.stop();
    update_thread.join();
    return 0;
}
