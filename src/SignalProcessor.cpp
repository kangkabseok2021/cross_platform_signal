#include "SignalProcessor.h"
#include <chrono>
#include <cmath>
#include <numeric>

SignalProcessor::SignalProcessor(ISignalSource& src, ISignalFilter& filter,
                                 std::optional<std::reference_wrapper<SqliteLogger>> logger)
    : src_(src), filter_(filter),
      fft_(src.sampleRate(), 1024), logger_(logger) {}

double SignalProcessor::rms(const std::vector<double>& v) {
    double sum = 0.0;
    for (double x : v) sum += x * x;
    return std::sqrt(sum / static_cast<double>(v.size()));
}

ProcessResult SignalProcessor::run(size_t n_samples) {
    auto t0 = std::chrono::steady_clock::now();

    auto raw      = src_.read(n_samples);
    auto filtered = filter_.apply(raw);
    auto fft_res  = fft_.analyse(raw);

    auto t1 = std::chrono::steady_clock::now();
    long long lat = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();

    ProcessResult res{
        raw, filtered,
        rms(raw), rms(filtered),
        fft_res.fundamental_hz, lat
    };

    if (logger_) {
        SqliteLogger::RunRecord rec{
            "synthetic", filter_.name(),
            n_samples, res.fundamental_hz,
            res.rms_raw, res.rms_filtered, lat
        };
        logger_->get().log(rec);
    }
    return res;
}
