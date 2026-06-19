#pragma once
#include <cmath>
#include <cstddef>

namespace anc {

enum class FaultType { NONE, MIC_DROPOUT, MIC_CLIP, ACTUATOR_DEGRADATION, DIVERGENCE };
enum class ANCState { ACTIVE, ADAPTATION_FROZEN, MUTED, PASSIVE };

struct FaultConfig {
    float dropout_threshold  = 1e-6f;   // sustained RMS below this → dropout
    float clip_threshold     = 0.99f;   // samples at/above this → clipping
    std::size_t clip_count   = 8;       // consecutive clipped samples to trigger
    float divergence_norm_sq = 1e4f;    // ||w||² above this → divergence
    std::size_t window       = 128;     // samples in detection window
};

class FaultDetector {
public:
    explicit FaultDetector(const FaultConfig& cfg = {}) : cfg_(cfg) {}

    // Update with one block of scalar signals.
    // ref, error: current sample values; weights_norm_sq: ||w||²
    void update(float ref, float error, float weights_norm_sq) noexcept {
        (void)ref;
        ++n_;

        // Clip detection
        if (std::fabs(error) >= cfg_.clip_threshold) ++clip_run_;
        else                                           clip_run_ = 0;

        // Energy accumulation
        energy_acc_ += error * error;

        if (n_ % cfg_.window == 0) {
            float rms = std::sqrt(energy_acc_ / cfg_.window);
            energy_acc_ = 0.0f;

            if (clip_run_ >= cfg_.clip_count)        fault_ = FaultType::MIC_CLIP;
            else if (rms < cfg_.dropout_threshold)   fault_ = FaultType::MIC_DROPOUT;
            else if (weights_norm_sq > cfg_.divergence_norm_sq)
                                                     fault_ = FaultType::DIVERGENCE;
            else                                     fault_ = FaultType::NONE;
        }

        // State machine
        switch (fault_) {
            case FaultType::NONE:                state_ = ANCState::ACTIVE; break;
            case FaultType::MIC_DROPOUT:         state_ = ANCState::MUTED;  break;
            case FaultType::MIC_CLIP:            state_ = ANCState::MUTED;  break;
            case FaultType::DIVERGENCE:          state_ = ANCState::ADAPTATION_FROZEN; break;
            case FaultType::ACTUATOR_DEGRADATION:state_ = ANCState::PASSIVE; break;
        }
    }

    FaultType fault()      const noexcept { return fault_; }
    ANCState  state()      const noexcept { return state_; }
    void      clear_fault() noexcept { fault_ = FaultType::NONE; state_ = ANCState::ACTIVE; }

private:
    FaultConfig cfg_;
    FaultType   fault_ = FaultType::NONE;
    ANCState    state_ = ANCState::ACTIVE;
    std::size_t n_         = 0;
    std::size_t clip_run_  = 0;
    float       energy_acc_ = 0.0f;
};

} // namespace anc
