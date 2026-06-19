#pragma once
#include "fxlms.hpp"
#include "imc.hpp"
#include "fault_detector.hpp"

namespace anc {

// Hybrid ANC engine: FxLMS feedforward (tonal) + IMC feedback (broadband).
//   W   = FxLMS adaptive filter taps
//   SP  = secondary path taps
//   IMC = IMC controller taps
template <std::size_t W, std::size_t SP, std::size_t IMCN>
class HybridANC {
public:
    HybridANC(float mu, float leakage,
              const SecondaryPath<SP>& sec_path,
              const typename IMCController<IMCN>::Coeffs& imc_coeffs)
        : fxlms_(mu, leakage, sec_path)
        , imc_(imc_coeffs)
    {}

    // Process one sample. Returns the total anti-noise cancellation signal.
    float process(float ref, float error) noexcept {
        float fault_wn = fxlms_.weights_norm_sq();
        fault_det_.update(ref, error, fault_wn);

        ANCState st = fault_det_.state();
        if (st == ANCState::MUTED)              return 0.0f;
        if (st == ANCState::PASSIVE)            return 0.0f;
        if (st == ANCState::ADAPTATION_FROZEN)  fxlms_.freeze();
        else                                    fxlms_.unfreeze();

        float y_ff = fxlms_.process(ref, error);
        float y_fb = imc_.process(error);
        return y_ff + y_fb;
    }

    FaultDetector&       fault_detector()       noexcept { return fault_det_; }
    const FaultDetector& fault_detector() const noexcept { return fault_det_; }
    FxLMS<W, SP>&        feedforward()          noexcept { return fxlms_; }
    IMCController<IMCN>& feedback()             noexcept { return imc_; }

    void reset() noexcept { fxlms_.reset(); imc_.reset(); fault_det_.clear_fault(); }

private:
    FxLMS<W, SP>       fxlms_;
    IMCController<IMCN> imc_;
    FaultDetector       fault_det_;
};

} // namespace anc
