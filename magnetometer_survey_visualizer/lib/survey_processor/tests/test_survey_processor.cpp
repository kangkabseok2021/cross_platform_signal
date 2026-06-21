#include <gtest/gtest.h>
#include "GradiometerSimulator.h"
#include "GaussianSmoother.h"
#include "BaselineSubtractor.h"
#include "RmsCalculator.h"
#include "AnomalyDetector.h"
#include "AnomalyExporter.h"
#include <set>
#include <tuple>
#include <cmath>
#include <fstream>
#include <sstream>
#include <numeric>
#include <algorithm>

// ── Task 2: Simulator ──────────────────────────────────────────────────────

TEST(Simulator, DipoleAnomalyPeakAboveNoiseFloor) {
    GradiometerSimulator sim(20, 20, 1.0f, 1, 42);
    sim.addDipole({10.0f, 10.0f, 0.5f, 100000.0f});
    auto pts = sim.generate();
    ASSERT_FALSE(pts.empty());

    std::vector<float> amps;
    amps.reserve(pts.size());
    for (const auto& p : pts) amps.push_back(p.amplitude_nT);

    float mean = std::accumulate(amps.begin(), amps.end(), 0.0f) / amps.size();
    float sq_sum = 0.0f;
    for (float a : amps) sq_sum += (a - mean) * (a - mean);
    float sigma = std::sqrt(sq_sum / amps.size());
    float peak = *std::max_element(amps.begin(), amps.end());

    EXPECT_GT(peak, mean + 5.0f * sigma);
}

TEST(Simulator, ZigZagCoversAllGridCells) {
    const int W = 8, H = 6, CH = 4;
    GradiometerSimulator sim(W, H, 1.0f, CH, 7);
    auto pts = sim.generate();

    EXPECT_EQ(static_cast<int>(pts.size()), W * H * CH);

    std::set<std::tuple<float, float, int>> seen;
    for (const auto& p : pts) {
        auto key = std::make_tuple(p.x, p.y, p.channel);
        EXPECT_EQ(seen.count(key), 0u) << "duplicate at x=" << p.x << " y=" << p.y << " ch=" << p.channel;
        seen.insert(key);
    }
    EXPECT_EQ(static_cast<int>(seen.size()), W * H * CH);
}

TEST(Simulator, DeterministicReplay) {
    GradiometerSimulator a(10, 10, 1.0f, 2, 123);
    a.addDipole({5.0f, 5.0f, 0.5f, 50000.0f});
    GradiometerSimulator b(10, 10, 1.0f, 2, 123);
    b.addDipole({5.0f, 5.0f, 0.5f, 50000.0f});

    auto pa = a.generate();
    auto pb = b.generate();
    ASSERT_EQ(pa.size(), pb.size());
    for (std::size_t i = 0; i < pa.size(); ++i) {
        EXPECT_EQ(pa[i].amplitude_nT, pb[i].amplitude_nT) << "mismatch at index " << i;
        EXPECT_EQ(pa[i].x, pb[i].x);
        EXPECT_EQ(pa[i].y, pb[i].y);
    }
}

// ── Tasks 3-4 tests added below ────────────────────────────────────────────
