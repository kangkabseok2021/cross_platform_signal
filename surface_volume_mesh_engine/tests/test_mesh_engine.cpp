#include <gtest/gtest.h>
#include "AdvancingFront2D.h"
#include "DelaunayMesh3D.h"
#include "MeshQuality.h"
#include "LaplacianSmoother.h"
#include "MeshExporter.h"
#include <cmath>
#include <filesystem>
#include <algorithm>

// ---- AFM 2D tests -------------------------------------------------------

TEST(AFM2D, ProducesTriangles) {
    std::vector<Point2D> sq = {{0,0},{1,0},{1,1},{0,1}};
    AdvancingFront2D afm(0.5);
    auto m = afm.mesh(sq);
    EXPECT_GE(m.tris.size(), 2u);
}

TEST(AFM2D, BoundaryNodesPreserved) {
    std::vector<Point2D> sq = {{0,0},{2,0},{2,2},{0,2}};
    AdvancingFront2D afm(0.6);
    auto m = afm.mesh(sq);
    for (auto& b : sq) {
        bool found = false;
        for (auto& n : m.nodes)
            if (std::hypot(n.x-b.x, n.y-b.y) < 1e-9) { found=true; break; }
        EXPECT_TRUE(found) << "Boundary node (" << b.x << "," << b.y << ") missing";
    }
}

TEST(AFM2D, NoInvertedElements) {
    std::vector<Point2D> sq = {{0,0},{1,0},{1,1},{0,1}};
    AdvancingFront2D afm(0.4);
    auto m = afm.mesh(sq);
    for (auto& t : m.tris) {
        double a = signed_area(m.nodes[t.vi[0]], m.nodes[t.vi[1]], m.nodes[t.vi[2]]);
        EXPECT_GT(a, 0.0) << "Inverted triangle found";
    }
}

TEST(AFM2D, TriangleCountScalesWithH) {
    std::vector<Point2D> sq = {{0,0},{4,0},{4,4},{0,4}};
    AdvancingFront2D coarse(1.0), fine(0.5);
    auto mc = coarse.mesh(sq);
    auto mf = fine.mesh(sq);
    EXPECT_GT(mf.tris.size(), mc.tris.size());
}

TEST(AFM2D, SnapDeduplication) {
    // Two triangle strips sharing a common interior edge should not duplicate the node
    std::vector<Point2D> sq = {{0,0},{2,0},{2,1},{0,1}};
    AdvancingFront2D afm(0.8);
    auto m = afm.mesh(sq);
    // All node positions should be unique (no duplicates within 1e-9)
    for (int i = 0; i < (int)m.nodes.size(); ++i)
        for (int j = i+1; j < (int)m.nodes.size(); ++j)
            EXPECT_GT(std::hypot(m.nodes[i].x-m.nodes[j].x,
                                  m.nodes[i].y-m.nodes[j].y), 1e-9);
}

TEST(AFM2D, NonEmptyForPolygon) {
    // Hexagonal boundary
    std::vector<Point2D> hex;
    for (int i = 0; i < 6; ++i)
        hex.push_back({std::cos(i*M_PI/3.0), std::sin(i*M_PI/3.0)});
    AdvancingFront2D afm(0.4);
    auto m = afm.mesh(hex);
    EXPECT_GE(m.tris.size(), 4u);
    for (auto& t : m.tris)
        EXPECT_GT(signed_area(m.nodes[t.vi[0]], m.nodes[t.vi[1]], m.nodes[t.vi[2]]), 0.0);
}
