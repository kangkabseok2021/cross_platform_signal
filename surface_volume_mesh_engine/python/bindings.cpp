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
            result["nodes"] = nodes;
            result["tris"]  = tris;
            result["quality"]   = quality;
            return result;
        }, py::arg("points"), py::arg("h"));

    mod.def("smooth_2d", [](py::dict mesh_dict, int boundary_count, int n_iter) -> py::dict {
        Mesh2D mesh;
        for (auto& pt : mesh_dict["nodes"].cast<py::list>()) {
            auto xy = pt.cast<py::list>();
            mesh.nodes.push_back({xy[0].cast<double>(), xy[1].cast<double>()});
        }
        for (auto& tri : mesh_dict["tris"].cast<py::list>()) {
            auto ijk = tri.cast<py::list>();
            mesh.tris.push_back({{{ijk[0].cast<int>(), ijk[1].cast<int>(), ijk[2].cast<int>()}}});
        }
        Mesh2D smoothed = laplacian_smooth_2d(std::move(mesh), boundary_count, n_iter);
        py::list nodes, tris;
        for (auto& n : smoothed.nodes) nodes.append(py::make_tuple(n.x, n.y));
        for (auto& t : smoothed.tris) tris.append(py::make_tuple(t.vi[0], t.vi[1], t.vi[2]));
        py::dict result;
        result["nodes"] = nodes;
        result["tris"] = tris;
        return result;
    });

    mod.def("quality_report_2d",
        [](py::dict mesh_dict) -> py::dict {
            Mesh2D mesh;
            for (auto& pt : mesh_dict["nodes"].cast<py::list>()) {
                auto xy = pt.cast<py::list>();
                mesh.nodes.push_back({xy[0].cast<double>(), xy[1].cast<double>()});
            }
            for (auto& tri : mesh_dict["tris"].cast<py::list>()) {
                auto ijk = tri.cast<py::list>();
                mesh.tris.push_back({{{ijk[0].cast<int>(), ijk[1].cast<int>(), ijk[2].cast<int>()}}});
            }
            auto qs = quality_report_2d(mesh);
            py::dict d;
            d["mean_aspect_ratio"]  = qs.mean_aspect_ratio;
            d["min_aspect_ratio"]   = qs.min_aspect_ratio;
            d["mean_min_angle_deg"] = qs.mean_min_angle_deg;
            d["min_angle_deg"]      = qs.min_angle_deg;
            d["frac_below_20deg"]   = qs.frac_below_20deg;
            return d;
        }, py::arg("mesh"));
}
