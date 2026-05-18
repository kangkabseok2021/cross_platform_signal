#pragma once
#include <vector>

// Abstract filter — apply FFT analysis, IIR, or moving average uniformly.
class ISignalFilter {
public:
    virtual ~ISignalFilter() = default;
    // Apply filter in-place semantics: input → output same length.
    virtual std::vector<double> apply(const std::vector<double>& in) = 0;
    virtual const char* name() const = 0;
};
