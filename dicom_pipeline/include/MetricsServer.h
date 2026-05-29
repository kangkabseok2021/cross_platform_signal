#pragma once
#include "Types.h"
#include <httplib.h>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

class MetricsServer {
public:
    MetricsServer() = default;
    ~MetricsServer();

    // Start background thread running HTTP server on specified port
    void start(uint16_t port);

    // Stop HTTP server and join thread
    void stop();

    // Record processing statistics into Prometheus metrics
    void record(const ProcessingStats& stats);

    // Set the service ready flag (first successful anonymization completed)
    void set_ready() noexcept;

private:
    void handle_metrics(const httplib::Request& req, httplib::Response& res);
    void handle_health(const httplib::Request& req, httplib::Response& res);
    void handle_ready(const httplib::Request& req, httplib::Response& res);

    std::atomic<bool> is_ready_{false};
    std::atomic<bool> running_{false};
    
    std::thread server_thread_;
    httplib::Server svr_;

    // Prometheus Metrics State
    std::mutex metrics_mutex_;
    uint64_t success_count_{0};
    uint64_t corrupted_count_{0};
    
    // Histogram buckets: {0.001, 0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 1.0}
    std::vector<double> bucket_thresholds_{0.001, 0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 1.0};
    std::vector<uint64_t> bucket_counts_{0, 0, 0, 0, 0, 0, 0, 0, 0}; // matched index-wise
    uint64_t inf_bucket_count_{0}; // counts all success
    double processing_seconds_sum_{0.0};
};
