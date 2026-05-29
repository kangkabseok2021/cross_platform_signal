#pragma once
#include "Types.h"
#include <ostream>
#include <mutex>

class ProcessingLogger {
public:
    explicit ProcessingLogger(std::ostream& out) noexcept;
    ~ProcessingLogger() = default;

    // Appends a single structured NDJSON line to the output stream
    void log(const ProcessingStats& stats);

    // Flushes the stream
    void flush();

private:
    std::ostream& out_;
    std::mutex mutex_;
};

