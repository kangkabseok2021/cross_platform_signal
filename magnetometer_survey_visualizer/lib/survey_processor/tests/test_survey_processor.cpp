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

// ── Task 3: Signal Processing ──────────────────────────────────────────────

TEST(SignalProcessing, GaussianSmootherSpreadsSingleSpike) {
    const int W = 5, H = 5;
    std::vector<float> grid(W * H, 0.0f);
    grid[2 * W + 2] = 100.0f;  // spike at (row=2, col=2)

    GaussianSmoother::apply(grid.data(), W, H);

    float peak = grid[2 * W + 2];
    EXPECT_LE(peak, 50.0f);

    // All 8 neighbours of (2,2) must be > 0
    const int nr[] = {1,1,1,2,2,3,3,3};
    const int nc[] = {1,2,3,1,3,1,2,3};
    for (int i = 0; i < 8; ++i)
        EXPECT_GT(grid[nr[i]*W + nc[i]], 0.0f) << "neighbour " << i << " is zero";
}

TEST(SignalProcessing, BaselineSubtractorZerosRowMeans) {
    const int W = 6, H = 4;
    std::vector<float> grid(W * H);
    for (int r = 0; r < H; ++r)
        for (int c = 0; c < W; ++c)
            grid[r * W + c] = static_cast<float>((r + 1) * 10);

    BaselineSubtractor::subtract(grid.data(), W, H);

    for (int r = 0; r < H; ++r) {
        float mean = 0.0f;
        for (int c = 0; c < W; ++c) mean += grid[r * W + c];
        mean /= W;
        EXPECT_NEAR(mean, 0.0f, 1e-5f) << "row " << r << " mean not zero";
    }
}

TEST(SignalProcessing, RmsMatchesManualFormula) {
    float grid[] = {3.0f, 4.0f, 0.0f, 0.0f};
    float rms = RmsCalculator::compute(grid, 2, 2);
    EXPECT_NEAR(rms, 2.5f, 1e-5f);
}

TEST(SignalProcessing, AnomalyDetectorFindsInjectedPeak) {
    const int W = 10, H = 10;
    std::vector<float> grid(W * H, 10.0f);
    grid[5 * W + 5] = 200.0f;

    auto candidates = AnomalyDetector::find(grid.data(), W, H, 100.0f);

    ASSERT_EQ(candidates.size(), 1u);
    EXPECT_EQ(candidates[0].grid_x, 5);
    EXPECT_EQ(candidates[0].grid_y, 5);
}

TEST(SignalProcessing, NonMaxSuppressionRetainsOnlyHigherPeak) {
    const int W = 10, H = 10;
    std::vector<float> grid(W * H, 0.0f);
    grid[4 * W + 4] = 150.0f;
    grid[5 * W + 5] = 200.0f;  // diagonal neighbour, strictly greater

    auto candidates = AnomalyDetector::find(grid.data(), W, H, 100.0f);

    ASSERT_EQ(candidates.size(), 1u);
    EXPECT_EQ(candidates[0].grid_x, 5);
    EXPECT_EQ(candidates[0].grid_y, 5);
}

// ── Task 4: AnomalyExporter ────────────────────────────────────────────────

TEST(Exporter, CsvContainsMandatoryColumns) {
    std::vector<AnomalyCandidate> anomalies = {
        {3, 7, 1.2f, 145.0f, 2},
        {5, 2, 0.8f, 220.0f, 0}
    };
    const std::string path = "/tmp/sensys_test_export.csv";
    ASSERT_TRUE(AnomalyExporter::exportCsv(anomalies, path));

    std::ifstream f(path);
    ASSERT_TRUE(f.is_open());
    std::string header;
    std::getline(f, header);
    EXPECT_NE(header.find("grid_x"), std::string::npos);
    EXPECT_NE(header.find("grid_y"), std::string::npos);
    EXPECT_NE(header.find("depth_m"), std::string::npos);
    EXPECT_NE(header.find("peak_nT"), std::string::npos);
    EXPECT_NE(header.find("channel"), std::string::npos);

    int data_lines = 0;
    std::string line;
    while (std::getline(f, line)) if (!line.empty()) ++data_lines;
    EXPECT_EQ(data_lines, 2);
}

TEST(Exporter, GeoJsonHasFeaturesArray) {
    std::vector<AnomalyCandidate> anomalies = {
        {4, 8, 1.5f, 310.0f, 1}
    };
    const std::string path = "/tmp/sensys_test_export.geojson";
    ASSERT_TRUE(AnomalyExporter::exportGeoJson(anomalies, 51.5, -0.1, 1.0, path));

    std::ifstream f(path);
    std::ostringstream ss;
    ss << f.rdbuf();
    std::string content = ss.str();
    EXPECT_NE(content.find("FeatureCollection"), std::string::npos);
    EXPECT_NE(content.find("features"), std::string::npos);
    EXPECT_NE(content.find("coordinates"), std::string::npos);
}
