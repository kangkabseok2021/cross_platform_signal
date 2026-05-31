#include <gtest/gtest.h>
#include <cmath>
#include <vector>

/* Include all headers — models are header-only or fully inlined for tests */
#include "AcousticEngine.h"
#include "GridDispatcher.h"
#include "InverseSquareLawModel.h"
#include "SabineReverbModel.h"

extern "C" {
#include "../legacy/acoustic.h"
}

/* ── Inverse square law physics ─────────────────────────────────────────── */

TEST(InverseSquareLaw, r1_dB_approx_Lw_minus_11) {
    InverseSquareLawModel m;
    /* Lp(r=1) = Lw - 20*log10(1) - 11 - 0.004*1 = Lw - 11.004 */
    const float expected = 80.0f - 11.0f - 0.004f;
    EXPECT_NEAR(m.computeSPL(80.0f, 1.0f), expected, 0.01f);
}

TEST(InverseSquareLaw, r10_dB_approx_Lw_minus_31) {
    InverseSquareLawModel m;
    /* Lp(r=10) = Lw - 20*log10(10) - 11 - 0.04 = Lw - 31.04 */
    const float expected = 80.0f - 20.0f - 11.0f - 0.004f * 10.0f;
    EXPECT_NEAR(m.computeSPL(80.0f, 10.0f), expected, 0.01f);
}

TEST(InverseSquareLaw, r100_dB_approx_Lw_minus_51) {
    InverseSquareLawModel m;
    /* Lp(r=100) = Lw - 40 - 11 - 0.4 = Lw - 51.4 */
    const float expected = 80.0f - 40.0f - 11.0f - 0.004f * 100.0f;
    EXPECT_NEAR(m.computeSPL(80.0f, 100.0f), expected, 0.01f);
}

TEST(InverseSquareLaw, ZeroRadius_NoNaNOrInf) {
    InverseSquareLawModel m;
    const float Lp = m.computeSPL(80.0f, 0.0f);  /* r clamped to 0.1 m */
    EXPECT_TRUE(std::isfinite(Lp));
}

/* ── Sabine reverberation model ─────────────────────────────────────────── */

TEST(SabineModel, RT60_Correct) {
    /* V=200 m³, A=40 sabins → T60 = 0.161*200/40 = 0.805 s */
    SabineReverbModel m(200.0f, 40.0f, 160.0f);
    EXPECT_NEAR(m.t60(), 0.805f, 0.001f);
}

TEST(SabineModel, SPL_IsFinite) {
    SabineReverbModel m(200.0f, 40.0f, 160.0f);
    EXPECT_TRUE(std::isfinite(m.computeSPL(80.0f, 10.0f)));
}

/* ── AcousticEngine — Strategy Pattern ──────────────────────────────────── */

TEST(AcousticEngine, Grid_Size_NxM) {
    AcousticEngine eng;
    std::vector<float> xs = {1.0f, 5.0f, 10.0f};
    std::vector<float> ys = {1.0f, 5.0f};
    const auto grid = eng.computeGrid(80.0f, 0.0f, 0.0f, xs, ys);
    EXPECT_EQ(grid.size(), xs.size() * ys.size());
}

TEST(AcousticEngine, Grid_AllFinite) {
    AcousticEngine eng;
    std::vector<float> xs = {1.0f, 5.0f, 10.0f, 20.0f, 50.0f};
    std::vector<float> ys = {1.0f, 5.0f, 10.0f, 20.0f, 50.0f};
    const auto grid = eng.computeGrid(80.0f, 0.0f, 0.0f, xs, ys);
    for (float v : grid) EXPECT_TRUE(std::isfinite(v));
}

TEST(AcousticEngine, StrategySwap_GridsDiffer) {
    AcousticEngine eng;
    std::vector<float> xs = {5.0f, 10.0f};
    std::vector<float> ys = {5.0f, 10.0f};
    const auto grid_isl  = eng.computeGrid(80.0f, 0.0f, 0.0f, xs, ys);

    eng.setModel(std::make_unique<SabineReverbModel>(200.0f, 40.0f, 160.0f));
    const auto grid_sab = eng.computeGrid(80.0f, 0.0f, 0.0f, xs, ys);

    /* Models produce different SPL fields */
    bool any_different = false;
    for (size_t i = 0; i < grid_isl.size(); ++i)
        if (std::abs(grid_isl[i] - grid_sab[i]) > 0.01f) { any_different = true; break; }
    EXPECT_TRUE(any_different);
}

TEST(AcousticEngine, StrategySwap_SwapBack_Restores) {
    AcousticEngine eng;
    std::vector<float> xs = {5.0f, 10.0f};
    std::vector<float> ys = {5.0f};
    const auto grid1 = eng.computeGrid(80.0f, 0.0f, 0.0f, xs, ys);

    eng.setModel(std::make_unique<SabineReverbModel>(200.0f, 40.0f, 160.0f));
    eng.setModel(std::make_unique<InverseSquareLawModel>());

    const auto grid2 = eng.computeGrid(80.0f, 0.0f, 0.0f, xs, ys);
    for (size_t i = 0; i < grid1.size(); ++i)
        EXPECT_NEAR(grid1[i], grid2[i], 1e-5f);
}

/* ── Legacy C parity ─────────────────────────────────────────────────────── */

TEST(LegacyCParity, MatchesWithin1e4dB) {
    /* 5×5 grid, Lw=80, source at origin */
    constexpr int   N   = 5;
    constexpr float Lw  = 80.0f;
    float coords[N] = {1.0f, 5.0f, 10.0f, 20.0f, 50.0f};

    /* C99 grid */
    float* c_grid = acoustic_alloc_grid(N, N);
    ASSERT_NE(c_grid, nullptr);
    ASSERT_EQ(compute_spl_grid(Lw, 0.0f, 0.0f, coords, coords, N, N, c_grid), 0);

    /* C++20 grid */
    AcousticEngine eng;
    std::vector<float> xs(coords, coords + N);
    std::vector<float> ys(coords, coords + N);
    const auto cpp_grid = eng.computeGrid(Lw, 0.0f, 0.0f, xs, ys);

    for (int i = 0; i < N * N; ++i)
        EXPECT_NEAR(c_grid[i], cpp_grid[static_cast<size_t>(i)], 1e-4f)
            << "Mismatch at index " << i;

    acoustic_free_grid(c_grid);
}

/* ── Parallel dispatcher ─────────────────────────────────────────────────── */

TEST(GridDispatcher, AsyncMatchesSerial) {
    InverseSquareLawModel model;
    std::vector<float> xs = {1.0f, 5.0f, 10.0f, 20.0f};
    std::vector<float> ys = {1.0f, 5.0f, 10.0f, 20.0f};

    const auto serial = [&]() {
        std::vector<float> g;
        g.reserve(xs.size() * ys.size());
        for (auto y : ys)
            for (auto x : xs) {
                const float r = std::sqrt(x*x + y*y);
                g.push_back(model.computeSPL(80.0f, r));
            }
        return g;
    }();

    const auto async_result = computeGridAsync(model, 80.0f, 0.0f, 0.0f, xs, ys);

    ASSERT_EQ(serial.size(), async_result.size());
    for (size_t i = 0; i < serial.size(); ++i)
        EXPECT_NEAR(serial[i], async_result[i], 1e-5f) << "Index " << i;
}

/* ── Compile-time constants ──────────────────────────────────────────────── */

TEST(ConstexprParams, AirAbsorption_1kHz) {
    static_assert(kAirAbsorption_1kHz == 0.004f,
                  "kAirAbsorption_1kHz must be 0.004 dB/m per ISO 9613-1");
    EXPECT_FLOAT_EQ(kAirAbsorption_1kHz, 0.004f);
}

/* ── Model metadata ──────────────────────────────────────────────────────── */

TEST(ModelMeta, ISL_NameNonEmpty) {
    InverseSquareLawModel m;
    EXPECT_FALSE(m.name().empty());
    EXPECT_EQ(m.name(), "InverseSquareLaw");
}

TEST(ModelMeta, Sabine_NameNonEmpty) {
    SabineReverbModel m(100.0f, 20.0f, 80.0f);
    EXPECT_FALSE(m.name().empty());
    EXPECT_EQ(m.name(), "SabineReverb");
}
