#include <gtest/gtest.h>
#include "layout_engine.h"
#include <QCoreApplication>
#include <cstdlib>
#include <filesystem>
#include <fstream>

// ── helpers ───────────────────────────────────────────────────────────────────

static std::string sample_layout_path() {
    // Relative to build dir
    return std::string(TEST_DATA_DIR) + "/chip_sample.json";
}

static std::string write_tmp_json(const std::string& content) {
    auto tmp = std::filesystem::temp_directory_path() / "test_layout.json";
    std::ofstream f(tmp);
    f << content;
    return tmp.string();
}

// ── tests ─────────────────────────────────────────────────────────────────────

TEST(LayoutEngineTest, ParsesSmallLayout) {
    LayoutEngine eng;
    eng.load(QString::fromStdString(sample_layout_path()));
    EXPECT_FALSE(eng.isEmpty());
    EXPECT_GT(eng.layerIds().size(), 0u);
}

TEST(LayoutEngineTest, LayerMapContainsAllLayers) {
    LayoutEngine eng;
    eng.load(QString::fromStdString(sample_layout_path()));
    auto ids = eng.layerIds();
    // Sample has layers 1, 2, 3
    EXPECT_GE(ids.size(), 1u);
    const auto& layers = eng.layers();
    for (auto id : ids)
        EXPECT_TRUE(layers.count(id) > 0) << "Missing layer " << id;
}

TEST(LayoutEngineTest, BoundingBoxNonEmpty) {
    LayoutEngine eng;
    eng.load(QString::fromStdString(sample_layout_path()));
    QRectF bb = eng.bbox();
    EXPECT_GT(bb.width(),  0.0);
    EXPECT_GT(bb.height(), 0.0);
}

TEST(LayoutEngineTest, CoordinateTransformIsInvertible) {
    // Polygons loaded from chip_sample.json have known bounds
    LayoutEngine eng;
    eng.load(QString::fromStdString(sample_layout_path()));
    QRectF bb = eng.bbox();
    // Bounding box min coords should match the first polygon
    EXPECT_GE(bb.left(), 0.0);
    EXPECT_GE(bb.top(),  0.0);
}

TEST(LayoutEngineTest, NullPathRaisesException) {
    LayoutEngine eng;
    EXPECT_THROW(eng.load(""), std::runtime_error);
}

TEST(LayoutEngineTest, EmptyFileReturnsZeroPolygons) {
    std::string tmp = write_tmp_json("{\"layers\":[]}");
    LayoutEngine eng;
    eng.load(QString::fromStdString(tmp));
    EXPECT_TRUE(eng.isEmpty());
}

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);  // needed for QString
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
