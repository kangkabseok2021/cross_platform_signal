#pragma once
#include <cstddef>
#include <vector>

// Abstract signal source — swap CSV file vs synthetic without touching filters.
class ISignalSource {
public:
    virtual ~ISignalSource() = default;
    // Return n samples. O(n) — exactly n samples always returned.
    virtual std::vector<double> read(size_t n) = 0;
    virtual double sampleRate() const = 0;
};
