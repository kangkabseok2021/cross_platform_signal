#pragma once
#include <array>
#include <cstddef>

class EmaFilter {
public:
    EmaFilter() noexcept : alpha_(0.1), s_(0.0) {}
    explicit EmaFilter(double alpha) noexcept
        : alpha_(alpha < 1e-9 ? 1e-9 : alpha > 1.0 ? 1.0 : alpha), s_(0.0) {}

    double process(double x) noexcept {
        s_ = alpha_ * x + (1.0 - alpha_) * s_;
        return s_;
    }

    void reset(double init = 0.0) noexcept { s_ = init; }

    [[nodiscard]] double alpha() const noexcept { return alpha_; }
    [[nodiscard]] double state() const noexcept { return s_; }

private:
    double alpha_;
    double s_;
};

template <std::size_t N>
class EmaFilterBank {
public:
    explicit EmaFilterBank(double alpha = 0.1) {
        for (auto& f : filters_) f = EmaFilter(alpha);
    }

    void setAlpha(std::size_t ch, double alpha) {
        if (ch < N) filters_[ch] = EmaFilter(alpha);
    }

    void processAll(const std::array<double, N>& inputs,
                    std::array<double, N>& outputs) noexcept {
        for (std::size_t i = 0; i < N; ++i)
            outputs[i] = filters_[i].process(inputs[i]);
    }

    void reset(std::size_t ch, double init = 0.0) {
        if (ch < N) filters_[ch].reset(init);
    }

    [[nodiscard]] double alpha(std::size_t ch) const noexcept {
        return ch < N ? filters_[ch].alpha() : 0.0;
    }

private:
    std::array<EmaFilter, N> filters_;
};
