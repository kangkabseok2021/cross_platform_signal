#pragma once
#include "ISignalFilter.h"
#include <deque>

// Simple moving average — O(N) using sliding window sum.
class MovingAverageFilter : public ISignalFilter {
public:
    explicit MovingAverageFilter(size_t window) : win_(window) {}

    std::vector<double> apply(const std::vector<double>& in) override {
        std::vector<double> out(in.size());
        double sum = 0.0;
        for (size_t i = 0; i < in.size(); ++i) {
            buf_.push_back(in[i]);
            sum += in[i];
            if (buf_.size() > win_) { sum -= buf_.front(); buf_.pop_front(); }
            out[i] = sum / static_cast<double>(buf_.size());
        }
        return out;
    }

    const char* name() const override { return "MovingAverage"; }

private:
    size_t win_;
    std::deque<double> buf_;
};
