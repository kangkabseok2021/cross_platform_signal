#include "control/MachineController.h"

MachineController::MachineController(SortingQueue<SortEvent, 64>& queue, int ejector_delay_ms)
    : queue_(queue), ejector_delay_ms_(ejector_delay_ms) {}

MachineController::~MachineController() {
    stop();
}

void MachineController::start() {
    if (!worker_.joinable()) {
        worker_ = std::jthread([this](std::stop_token st) { run(st); });
    }
}

void MachineController::stop() {
    worker_.request_stop();
    if (worker_.joinable()) {
        worker_.join();
    }
}

int64_t MachineController::get_sorted_count() const {
    return sorted_count_.load(std::memory_order_relaxed);
}

void MachineController::run(std::stop_token stop_token) {
    while (!stop_token.stop_requested()) {
        SortEvent event;
        if (queue_.pop(event)) {
            // Simulate pneumatic ejector delay
            std::this_thread::sleep_for(std::chrono::milliseconds(ejector_delay_ms_));
            sorted_count_.fetch_add(1, std::memory_order_relaxed);
        } else {
            // Short backoff to avoid 100% CPU usage
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}
