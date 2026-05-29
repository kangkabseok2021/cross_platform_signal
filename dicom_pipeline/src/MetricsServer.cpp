#include "MetricsServer.h"
#include <sstream>
#include <iomanip>

MetricsServer::~MetricsServer() {
    stop();
}

void MetricsServer::start(uint16_t port) {
    if (running_) {
        return;
    }
    running_ = true;

    server_thread_ = std::thread([this, port]() {
        svr_.Get("/metrics", [this](const httplib::Request& req, httplib::Response& res) {
            handle_metrics(req, res);
        });
        svr_.Get("/health", [this](const httplib::Request& req, httplib::Response& res) {
            handle_health(req, res);
        });
        svr_.Get("/ready", [this](const httplib::Request& req, httplib::Response& res) {
            handle_ready(req, res);
        });
        svr_.listen("0.0.0.0", port);
    });
}

void MetricsServer::stop() {
    if (!running_) {
        return;
    }
    running_ = false;
    svr_.stop();
    if (server_thread_.joinable()) {
        server_thread_.join();
    }
}

void MetricsServer::record(const ProcessingStats& stats) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    if (stats.status == "success") {
        success_count_++;
        double seconds = static_cast<double>(stats.duration_us) / 1000000.0;
        processing_seconds_sum_ += seconds;

        // Categorize into the first bucket it fits
        bool fit = false;
        for (size_t i = 0; i < bucket_thresholds_.size(); ++i) {
            if (seconds <= bucket_thresholds_[i]) {
                bucket_counts_[i]++;
                fit = true;
                break;
            }
        }
        if (!fit) {
            inf_bucket_count_++;
        }
    } else {
        corrupted_count_++;
    }
}

void MetricsServer::set_ready() noexcept {
    is_ready_ = true;
}

void MetricsServer::handle_health(const httplib::Request&, httplib::Response& res) {
    res.status = 200;
    res.set_content("{\"status\":\"ok\"}", "application/json");
}

void MetricsServer::handle_ready(const httplib::Request&, httplib::Response& res) {
    if (is_ready_) {
        res.status = 200;
        res.set_content("{\"status\":\"ready\"}", "application/json");
    } else {
        res.status = 503;
        res.set_content("{\"status\":\"initializing\"}", "application/json");
    }
}

void MetricsServer::handle_metrics(const httplib::Request&, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    std::stringstream ss;
    ss << "# HELP dicom_files_processed_total Total DICOM files processed by result\n";
    ss << "# TYPE dicom_files_processed_total counter\n";
    ss << "dicom_files_processed_total{result=\"success\"} " << success_count_ << "\n";
    ss << "dicom_files_processed_total{result=\"corrupted\"} " << corrupted_count_ << "\n";

    ss << "# HELP dicom_processing_seconds File processing duration\n";
    ss << "# TYPE dicom_processing_seconds histogram\n";

    uint64_t cumulative = 0;
    for (size_t i = 0; i < bucket_thresholds_.size(); ++i) {
        cumulative += bucket_counts_[i];
        ss << "dicom_processing_seconds_bucket{le=\"" << bucket_thresholds_[i] << "\"} " << cumulative << "\n";
    }
    ss << "dicom_processing_seconds_bucket{le=\"+Inf\"} " << success_count_ << "\n";
    
    ss << std::fixed << std::setprecision(6);
    ss << "dicom_processing_seconds_sum " << processing_seconds_sum_ << "\n";
    ss << "dicom_processing_seconds_count " << success_count_ << "\n";

    res.status = 200;
    res.set_content(ss.str(), "text/plain; version=0.0.4");
}
