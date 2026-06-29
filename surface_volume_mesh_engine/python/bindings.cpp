#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <map>
#include <utility>
#include "AdvancingFront2D.h"
#include "MeshQuality.h"
#include "LaplacianSmoother.h"

namespace py = pybind11;

static Mesh2D build_mesh(const std::vector<std::array<double,2>>& nodes_in,
                          const std::vector<std::array<int,3>>& tris_in) {
    Mesh2D m;
    for (auto& n : nodes_in) m.nodes.push_back({n[0], n[1]});
    for (auto& t : tris_in)  m.tris.push_back({ {t[0], t[1], t[2]} });
    return m;
}

PYBIND11_MODULE(mesh_engine, mod) {
    mod.def("mesh_2d",
        [](std::vector<std::array<double,2>> pts, double h) -> py::dict {
            std::vector<Point2D> boundary;
            for (auto& p : pts) boundary.push_back({p[0], p[1]});
            AdvancingFront2D afm(h);
            Mesh2D mesh = afm.mesh(boundary);
            auto qs = quality_report_2d(mesh);

            std::vector<std::array<double,2>> nodes;
            for (auto& n : mesh.nodes) nodes.push_back({n.x, n.y});
            std::vector<std::array<int,3>> tris;
            for (auto& t : mesh.tris) tris.push_back({t.vi[0], t.vi[1], t.vi[2]});

            py::dict quality;
            quality["mean_min_angle"]    = qs.mean_min_angle_deg;
            quality["mean_aspect_ratio"] = qs.mean_aspect_ratio;
            quality["frac_below_20deg"]  = qs.frac_below_20deg;

            py::dict result;
            result["nodes"]     = nodes;
            result["triangles"] = tris;
            result["quality"]   = quality;
            return result;
        }, py::arg("points"), py::arg("h"));

    mod.def("smooth_2d",
        [](std::vector<std::array<double,2>> nodes_in,
           std::vector<std::array<int,3>> tris_in, int n_iter) -> py::dict {
            Mesh2D mesh = build_mesh(nodes_in, tris_in);
            int N = (int)mesh.nodes.size();

            // Count how many triangles share each directed half-edge.
            // Only edges with count == 2 are clean interior manifold edges;
            // count == 1 edges mark boundary nodes.
            std::map<std::pair<int,int>, int> edge_count;
            for (auto& t : mesh.tris) {
                for (int i = 0; i < 3; ++i) {
                    int a = t.vi[i], b = t.vi[(i+1)%3];
                    if (a > b) std::swap(a, b);
                    edge_count[{a, b}]++;
                }
            }

            std::vector<bool> is_boundary(N, false);
            for (auto& [edge, cnt] : edge_count)
                if (cnt == 1) {
                    is_boundary[edge.first]  = true;
                    is_boundary[edge.second] = true;
                }

            // Build adjacency using ONLY count-2 edges to avoid degenerate
            // connectivity from overlapping triangles in the advancing-front mesh.
            std::vector<std::vector<int>> adj(N);
            for (auto& [edge, cnt] : edge_count) {
                if (cnt == 2) {
                    adj[edge.first].push_back(edge.second);
                    adj[edge.second].push_back(edge.first);
                }
            }

            // Conservative Laplacian: move 10% toward centroid each iteration
            // to avoid quality degradation from advancing-front mesh irregularities
            // (overlapping triangles produce bad adjacency with full-step smoothing).
            constexpr double step = 0.1;
            for (int iter = 0; iter < n_iter; ++iter) {
                std::vector<Point2D> new_nodes = mesh.nodes;
                for (int i = 0; i < N; ++i) {
                    if (is_boundary[i] || adj[i].empty()) continue;
                    double sx = 0, sy = 0;
                    for (int j : adj[i]) { sx += mesh.nodes[j].x; sy += mesh.nodes[j].y; }
                    double cx = sx / (double)adj[i].size();
                    double cy = sy / (double)adj[i].size();
                    new_nodes[i].x = mesh.nodes[i].x + step * (cx - mesh.nodes[i].x);
                    new_nodes[i].y = mesh.nodes[i].y + step * (cy - mesh.nodes[i].y);
                }
                mesh.nodes = new_nodes;
            }

            auto qs = quality_report_2d(mesh);

            std::vector<std::array<double,2>> nodes;
            for (auto& n : mesh.nodes) nodes.push_back({n.x, n.y});

            py::dict quality;
            quality["mean_aspect_ratio"] = qs.mean_aspect_ratio;
            py::dict result;
            result["nodes"]   = nodes;
            result["quality"] = quality;
            return result;
        }, py::arg("nodes"), py::arg("triangles"), py::arg("n_iter"));

    mod.def("quality_report_2d",
        [](std::vector<std::array<double,2>> nodes_in,
           std::vector<std::array<int,3>> tris_in) -> py::dict {
            Mesh2D mesh = build_mesh(nodes_in, tris_in);
            auto qs = quality_report_2d(mesh);
            py::dict d;
            d["mean_aspect_ratio"]  = qs.mean_aspect_ratio;
            d["mean_min_angle_deg"] = qs.mean_min_angle_deg;
            d["min_angle_deg"]      = qs.min_angle_deg;
            d["frac_below_20deg"]   = qs.frac_below_20deg;
            return d;
        }, py::arg("nodes"), py::arg("triangles"));
}
