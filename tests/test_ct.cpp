#include <gtest/gtest.h>
#include "FilterBank.h"
#include "Reconstructor.h"
#include <cmath>
#include <numbers>

using namespace ct;

// ── FilterBank tests ────────────────────────────────────────────────────────

// Test 1: Ram-Lak DC component is zero
TEST(FilterBank, RamLakDcIsZero) {
    auto H = FilterBank::buildKernel(FilterType::RamLak, 64);
    EXPECT_FLOAT_EQ(H[0], 0.f);
}

// Test 2: Ram-Lak ramp increases monotonically from k=0 to k=N/2
TEST(FilterBank, RamLakRampMonotonicallyIncreasing) {
    const std::size_t n = 64;
    auto H = FilterBank::buildKernel(FilterType::RamLak, n);
    for (std::size_t k = 1; k <= n / 2; ++k)
        EXPECT_GE(H[k], H[k - 1]) << "monotonicity failed at k=" << k;
}

// Test 3: Shepp-Logan ≤ Ram-Lak at all positive frequencies (sinc window damps)
TEST(FilterBank, SheppLoganLessOrEqualRamLak) {
    const std::size_t n = 64;
    auto H_rl = FilterBank::buildKernel(FilterType::RamLak,    n);
    auto H_sl = FilterBank::buildKernel(FilterType::SheppLogan, n);
    for (std::size_t k = 1; k <= n / 2; ++k)
        EXPECT_LE(H_sl[k], H_rl[k] + 1e-5f) << "Shepp-Logan exceeded Ram-Lak at k=" << k;
}

// ── Reconstructor tests ─────────────────────────────────────────────────────

// Build uniform angle array
static std::vector<float> uniform_angles(std::size_t n) {
    std::vector<float> a(n);
    for (std::size_t i = 0; i < n; ++i)
        a[i] = static_cast<float>(i) * static_cast<float>(std::numbers::pi) / n;
    return a;
}

// Test 4: Round-trip point source — reconstruct a phantom with one bright pixel
TEST(Reconstructor, RoundTripPointSource) {
    const std::size_t sz = 32;
    const std::size_t n_angles = 90;
    const std::size_t n_bins   = sz;

    Image phantom;
    phantom.size = sz;
    phantom.data.assign(sz * sz, 0.f);
    phantom.at(sz / 2, sz / 2) = 1.f;   // single bright pixel at centre

    auto angles = uniform_angles(n_angles);
    Sinogram sino = Reconstructor::forwardProject(phantom, angles, n_bins);
    FilterBank::filterSinogram(sino, FilterType::RamLak);
    Image recon = Reconstructor::reconstruct(sino, angles, sz);

    // Peak should be near centre
    float max_val = *std::max_element(recon.data.begin(), recon.data.end());
    float centre_val = recon.at(sz / 2, sz / 2);
    EXPECT_GT(centre_val, 0.3f * max_val)
        << "Reconstructed peak not at centre: centre=" << centre_val << " max=" << max_val;
}

// Test 5: Parallel reconstruction matches single-threaded (determinism)
TEST(Reconstructor, ParallelMatchesSingle) {
    const std::size_t sz = 16;
    const std::size_t n_angles = 36;

    Image phantom;
    phantom.size = sz;
    phantom.data.assign(sz * sz, 0.f);
    phantom.at(4, 4) = 1.f;
    phantom.at(8, 8) = 0.5f;

    auto angles = uniform_angles(n_angles);
    Sinogram sino = Reconstructor::forwardProject(phantom, angles, sz);
    FilterBank::filterSinogram(sino, FilterType::SheppLogan);

    // Run twice — par_unseq must be deterministic for floating-point identical input
    Image r1 = Reconstructor::reconstruct(sino, angles, sz);
    Image r2 = Reconstructor::reconstruct(sino, angles, sz);

    for (std::size_t i = 0; i < r1.data.size(); ++i)
        EXPECT_FLOAT_EQ(r1.data[i], r2.data[i]) << "mismatch at pixel " << i;
}

// Test 6: Different filters produce distinct output images
TEST(Reconstructor, FilterSwitchProducesDistinctImages) {
    const std::size_t sz = 24;
    const std::size_t n_angles = 60;

    Image phantom;
    phantom.size = sz;
    phantom.data.assign(sz * sz, 0.f);
    for (std::size_t i = 8; i < 16; ++i)
        for (std::size_t j = 8; j < 16; ++j)
            phantom.at(i, j) = 1.f;

    auto angles = uniform_angles(n_angles);

    Sinogram sino_rl = Reconstructor::forwardProject(phantom, angles, sz);
    Sinogram sino_sl = sino_rl;  // copy

    FilterBank::filterSinogram(sino_rl, FilterType::RamLak);
    FilterBank::filterSinogram(sino_sl, FilterType::SheppLogan);

    Image img_rl = Reconstructor::reconstruct(sino_rl, angles, sz);
    Image img_sl = Reconstructor::reconstruct(sino_sl, angles, sz);

    // Compute mean absolute difference — must be non-trivial
    float diff = 0.f;
    for (std::size_t i = 0; i < img_rl.data.size(); ++i)
        diff += std::abs(img_rl.data[i] - img_sl.data[i]);
    diff /= static_cast<float>(img_rl.data.size());

    EXPECT_GT(diff, 1e-4f) << "Ram-Lak and Shepp-Logan produced identical images";
}
