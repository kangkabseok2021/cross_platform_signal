#include "inference/CameraSimulator.h"
#include <thread>
#include <chrono>

CameraSimulator::CameraSimulator(double /*fps*/) : current_frame_id_(0) {}

int CameraSimulator::next_frame() {
    // Simulated frame capture delay (1ms)
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return ++current_frame_id_;
}

int CameraSimulator::get_current_frame_id() const {
    return current_frame_id_;
}
