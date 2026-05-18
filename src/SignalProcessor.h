#pragma once
#include "ISignalSource.h"
#include "ISignalFilter.h"
#include "FftAnalyser.h"
#include "SqliteLogger.h"
#include <memory>
#include <optional>

struct ProcessResult {
    std::vector<double> raw;
    std::vector<double> filtered;
    double rms_raw;
    double rms_filtered;
    double fundamental_hz;
    long long latency_us;
};

// Orchestrates source → filter → log in one call.
class SignalProcessor {
public:
    SignalProcessor(ISignalSource& src, ISignalFilter& filter,
                    std::optional<std::reference_wrapper<SqliteLogger>> logger = {});

    ProcessResult run(size_t n_samples);

private:
    ISignalSource& src_;
    ISignalFilter& filter_;
    FftAnalyser    fft_;
    std::optional<std::reference_wrapper<SqliteLogger>> logger_;

    static double rms(const std::vector<double>& v);
};
