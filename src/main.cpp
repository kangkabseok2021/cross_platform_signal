#include "SyntheticSource.h"
#include "FftAnalyser.h"
#include "ButterworthFilter.h"
#include "MovingAverageFilter.h"
#include "SignalProcessor.h"
#include "SqliteLogger.h"
#include <iostream>
#include <optional>

int main() {
    constexpr double SR     = 1000.0;
    constexpr double FREQ   =  50.0;
    constexpr size_t N      = 1024;

    SyntheticSource   src(FREQ, 1.0, 0.1, SR);
    ButterworthFilter filt(150.0, SR);

    SqliteLogger logger("signal_runs.db");
    SignalProcessor proc(src, filt, std::ref(logger));

    auto result = proc.run(N);

    std::cout << "Source:         synthetic 50 Hz + noise\n"
              << "Filter:         Butterworth low-pass 150 Hz\n"
              << "Samples:        " << N << "\n"
              << "RMS raw:        " << result.rms_raw      << "\n"
              << "RMS filtered:   " << result.rms_filtered << "\n"
              << "Fundamental Hz: " << result.fundamental_hz << "\n"
              << "Latency µs:     " << result.latency_us   << "\n";
    return 0;
}
