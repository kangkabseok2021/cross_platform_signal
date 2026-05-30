#include "SensorSimulator.h"
#include <random>

VehicleSensorSimulator::VehicleSensorSimulator()
    : rng_(std::random_device{}())
{
    channels_[0] = {"engine_temp",  "°C",  80.0, 80.0,  60.0, 120.0, 115.0, false};
    channels_[1] = {"battery_v",   "V",   12.8, 12.8,  11.5,  14.5,  11.8, false};
    channels_[2] = {"oil_pressure","bar",   3.5,  3.5,   2.0,   5.0,   2.2, false};
    channels_[3] = {"rpm",         "RPM", 2000.0, 2000.0, 800.0, 4000.0, 4500.0, false};

    for (std::size_t i = 0; i < NUM_CHANNELS; ++i)
        baselines_[i] = channels_[i].raw;
}

void VehicleSensorSimulator::update() {
    const double sigmas[NUM_CHANNELS] = {3.0, 0.15, 0.2, 30.0};
    for (std::size_t i = 0; i < NUM_CHANNELS; ++i) {
        std::normal_distribution<double> noise(0.0, sigmas[i]);
        double raw = baselines_[i] + noise(rng_) + fault_spike_[i];
        fault_spike_[i] = 0.0;
        channels_[i].raw = raw;
        if (i == 0) channels_[i].fault = raw > channels_[i].fault_threshold;
        else if (i == 1) channels_[i].fault = raw < channels_[i].fault_threshold;
        else if (i == 2) channels_[i].fault = raw < channels_[i].fault_threshold;
        else channels_[i].fault = raw > channels_[i].fault_threshold;
    }
}

void VehicleSensorSimulator::injectFault(int channel, double spike_magnitude) {
    if (channel >= 0 && channel < static_cast<int>(NUM_CHANNELS))
        fault_spike_[static_cast<std::size_t>(channel)] = spike_magnitude;
}

const std::array<SensorChannel, VehicleSensorSimulator::NUM_CHANNELS>&
VehicleSensorSimulator::channels() const noexcept {
    return channels_;
}

std::array<double, VehicleSensorSimulator::NUM_CHANNELS>
VehicleSensorSimulator::rawValues() const noexcept {
    std::array<double, NUM_CHANNELS> vals{};
    for (std::size_t i = 0; i < NUM_CHANNELS; ++i)
        vals[i] = channels_[i].raw;
    return vals;
}
