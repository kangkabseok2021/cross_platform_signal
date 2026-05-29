#ifndef MACHINE_CONTROLLER_H
#define MACHINE_CONTROLLER_H

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include "control/SortingQueue.h"

struct SortEvent {
    std::string material;
    float confidence;
    float x, y, w, h;
    std::chrono::steady_clock::time_point timestamp;
};

class MachineController {
private:
    SortingQueue<SortEvent, 64>& queue_;
    std::atomic<int64_t> sorted_count_{0};
    std::jthread worker_;
    int ejector_delay_ms_;

    void run(std::stop_token stop_token);
public:
    MachineController(SortingQueue<SortEvent, 64>& queue, int ejector_delay_ms = 50);
    ~MachineController();
    void start();
    void stop();
    int64_t get_sorted_count() const;
};

#endif // MACHINE_CONTROLLER_H
