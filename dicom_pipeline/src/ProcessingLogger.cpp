#include "ProcessingLogger.h"
#include <nlohmann/json.hpp>

ProcessingLogger::ProcessingLogger(std::ostream& out) noexcept
    : out_(out) {}

void ProcessingLogger::log(const ProcessingStats& stats) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    double throughput_mb_per_s = 0.0;
    if (stats.duration_us > 0) {
        throughput_mb_per_s = static_cast<double>(stats.file_size_bytes) / stats.duration_us;
    }

    nlohmann::json j;
    j["ts_ns"] = stats.ts_ns;
    j["file_size_bytes"] = stats.file_size_bytes;
    j["duration_us"] = stats.duration_us;
    j["throughput_mb_per_s"] = throughput_mb_per_s;
    j["status"] = stats.status;
    j["tags_scrubbed"] = stats.tags_scrubbed;

    out_ << j.dump() << "\n";
    out_.flush();
}

void ProcessingLogger::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    out_.flush();
}

