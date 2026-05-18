#include <gtest/gtest.h>
#include "../src/SyntheticSource.h"
#include "../src/CsvFileSource.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>

TEST(SyntheticSourceTest, ReturnsCorrectCount) {
    SyntheticSource src(50.0, 1.0, 0.0, 1000.0, 0);
    auto v = src.read(256);
    EXPECT_EQ(v.size(), 256u);
}

TEST(SyntheticSourceTest, SampleRateReturned) {
    SyntheticSource src(50.0, 1.0, 0.0, 2000.0, 0);
    EXPECT_EQ(src.sampleRate(), 2000.0);
}

TEST(SyntheticSourceTest, ZeroNoiseFirstSampleIsZero) {
    SyntheticSource src(100.0, 2.0, 0.0, 1000.0, 0);
    auto v = src.read(1);
    // t=0: A·sin(0) = 0
    EXPECT_NEAR(v[0], 0.0, 1e-10);
}

TEST(SyntheticSourceTest, AmplitudeRespected) {
    SyntheticSource src(1.0, 3.0, 0.0, 1000.0, 0);
    auto v = src.read(1000);
    double mx = *std::max_element(v.begin(), v.end());
    EXPECT_NEAR(mx, 3.0, 0.01);
}

TEST(SyntheticSourceTest, SequentialReadsAreContinuous) {
    SyntheticSource src(10.0, 1.0, 0.0, 1000.0, 0);
    auto a = src.read(100);
    auto b = src.read(100);
    EXPECT_NE(a[0], b[0]);
}

TEST(CsvFileSourceTest, ReadsTwoValues) {
    auto tmp = std::filesystem::temp_directory_path() / "test_sig.csv";
    { std::ofstream f(tmp); f << "1.5\n2.5\n3.5\n"; }
    std::vector<double> v;
    {
        CsvFileSource csv(tmp.string(), 1000.0);
        v = csv.read(2);
    }  // csv destroyed here — ifstream closed, file lock released on Windows
    std::filesystem::remove(tmp);
    EXPECT_EQ(v.size(), 2u);
    EXPECT_NEAR(v[0], 1.5, 1e-9);
    EXPECT_NEAR(v[1], 2.5, 1e-9);
}

TEST(CsvFileSourceTest, PadsWithZerosWhenExhausted) {
    auto tmp = std::filesystem::temp_directory_path() / "test_short.csv";
    { std::ofstream f(tmp); f << "9.0\n"; }
    std::vector<double> v;
    {
        CsvFileSource csv(tmp.string(), 1000.0);
        v = csv.read(3);
    }  // csv destroyed — file released before remove
    std::filesystem::remove(tmp);
    EXPECT_EQ(v.size(), 3u);
    EXPECT_NEAR(v[0], 9.0, 1e-9);
    EXPECT_EQ(v[1], 0.0);
    EXPECT_EQ(v[2], 0.0);
}
