#pragma once
#include <array>
#include <random>
#include <string>

struct SensorChannel {
    std::string name;
    std::string unit;
    double raw{};
    double filtered{};
    double min_val{};
    double max_val{};
    double fault_threshold{};
    bool fault{false};
};

class VehicleSensorSimulator {
public:
    static constexpr std::size_t NUM_CHANNELS = 4;

    VehicleSensorSimulator();
    void update();
    void injectFault(int channel, double spike_magnitude);

    [[nodiscard]] const std::array<SensorChannel, NUM_CHANNELS>& channels() const noexcept;
    [[nodiscard]] std::array<double, NUM_CHANNELS> rawValues() const noexcept;

private:
    std::mt19937 rng_;
    std::array<SensorChannel, NUM_CHANNELS> channels_;
    std::array<double, NUM_CHANNELS> baselines_{};
    std::array<double, NUM_CHANNELS> fault_spike_{};
};
