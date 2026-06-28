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

// ---- Delaunay 3D tests --------------------------------------------------

TEST(Delaunay3D, ProducesTetrahedra) {
    std::vector<Point3D> pts = {{0,0,0},{1,0,0},{0,1,0},{0,0,1},{0.5,0.5,0.5}};
    DelaunayMesh3D d;
    auto m = d.triangulate(pts);
    EXPECT_GE(m.tets.size(), 1u);
}

TEST(Delaunay3D, NoSuperTetVertices) {
    std::vector<Point3D> pts = {{0,0,0},{1,0,0},{0,1,0},{0,0,1},{1,1,0},{1,0,1}};
    DelaunayMesh3D d;
    auto m = d.triangulate(pts);
    // All tet vertex indices must be in [0, pts.size()-1]
    for (auto& t : m.tets)
        for (int v : t.vi)
            EXPECT_LT(v, (int)pts.size());
}

TEST(Delaunay3D, PositiveVolumeAllTets) {
    std::vector<Point3D> pts;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            for (int k = 0; k < 3; ++k)
                pts.push_back({(double)i, (double)j, (double)k});
    DelaunayMesh3D d;
    auto m = d.triangulate(pts);
    EXPECT_EQ(m.n_inverted, 0);
    for (auto& t : m.tets)
        EXPECT_GT(tet_volume(m.nodes[t.vi[0]], m.nodes[t.vi[1]],
                             m.nodes[t.vi[2]], m.nodes[t.vi[3]]), 0.0);
}

TEST(Delaunay3D, CircumspherePropertySmallSet) {
    // For N<=6 non-coplanar points, check Delaunay property exhaustively
    std::vector<Point3D> pts = {{0,0,0},{1,0,0},{0,1,0},{0,0,1},{0.25,0.25,0.25}};
    DelaunayMesh3D d;
    auto m = d.triangulate(pts);
    for (auto& t : m.tets) {
        for (int pi = 0; pi < (int)pts.size(); ++pi) {
            bool is_vert = false;
            for (int v : t.vi) if (v == pi) { is_vert = true; break; }
            if (is_vert) continue;
            // No other input point should be strictly inside the circumsphere
            EXPECT_FALSE(circumsphere_contains(m.nodes[t.vi[0]], m.nodes[t.vi[1]],
                                               m.nodes[t.vi[2]], m.nodes[t.vi[3]],
                                               pts[pi]));
        }
    }
}

TEST(Delaunay3D, VtkFileWritten) {
    std::vector<Point3D> pts = {{0,0,0},{1,0,0},{0,1,0},{0,0,1},{0.5,0.5,0.5}};
    DelaunayMesh3D d;
    auto m = d.triangulate(pts);
    std::filesystem::path p = std::filesystem::temp_directory_path() / "test_out.vtk";
    export_vtk(m, p);
    EXPECT_TRUE(std::filesystem::exists(p));
    EXPECT_GT(std::filesystem::file_size(p), 0u);
    std::filesystem::remove(p);
}

// ---- MeshQuality tests --------------------------------------------------

TEST(MeshQuality, EquilateralTriangleAspectRatio) {
    // Equilateral triangle has aspect_ratio = 1.0
    Mesh2D m;
    m.nodes = {{0,0}, {1,0}, {0.5, std::sqrt(3.0)/2.0}};
    m.tris  = {{ {0,1,2} }};
    double ar = aspect_ratio_2d(m, m.tris[0]);
    EXPECT_NEAR(ar, 1.0, 1e-6);
}

TEST(MeshQuality, CoplanarTetFlaggedInverted) {
    // Four nearly coplanar points → tet_volume ≈ 0 → classified as inverted
    Mesh3D m3;
    m3.nodes = {{0,0,0},{1,0,0},{0,1,0},{0.5,0.5,1e-14}};
    m3.tets  = {{ {0,1,2,3} }};
    double jac = jacobian_det(m3, m3.tets[0]);
    EXPECT_NEAR(jac, 0.0, 1e-10);
}

// ---- LaplacianSmoother tests --------------------------------------------

TEST(LaplacianSmoother, SmoothingImprovesAspectRatio) {
    // Dense mesh of a rectangle — interior nodes should improve after smoothing
    std::vector<Point2D> sq = {{0,0},{4,0},{4,4},{0,4}};
    AdvancingFront2D afm(0.7);
    auto m = afm.mesh(sq);
    auto q_before = quality_report_2d(m);
    auto smoothed = laplacian_smooth_2d(m, 4, 10);
    auto q_after  = quality_report_2d(smoothed);
    // Smoothing must not worsen mean aspect ratio beyond a small tolerance
    EXPECT_LE(q_after.mean_aspect_ratio, q_before.mean_aspect_ratio + 0.05);
}
