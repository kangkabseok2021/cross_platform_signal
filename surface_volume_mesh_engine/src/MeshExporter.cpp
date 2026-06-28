#include "MeshExporter.h"
#include <fstream>
#include <stdexcept>

void export_obj(const Mesh2D& mesh, const std::filesystem::path& path) {
    std::ofstream f(path);
    if (!f) throw std::runtime_error("Cannot open: " + path.string());
    f << "# surface mesh nodes=" << mesh.nodes.size()
      << " tris=" << mesh.tris.size() << "\n";
    for (auto& n : mesh.nodes)
        f << "v " << n.x << " " << n.y << " 0.0\n";
    for (auto& t : mesh.tris)
        f << "f " << t.vi[0]+1 << " " << t.vi[1]+1 << " " << t.vi[2]+1 << "\n";
}

void export_vtk(const Mesh3D& mesh, const std::filesystem::path& path) {
    std::ofstream out(path);
    out << "# vtk DataFile Version 2.0\n";
    out << "Mesh3D\n";
    out << "ASCII\n";
    out << "DATASET UNSTRUCTURED_GRID\n";
    out << "POINTS " << mesh.nodes.size() << " double\n";
    for (auto& p : mesh.nodes)
        out << p.x << " " << p.y << " " << p.z << "\n";
    out << "CELLS " << mesh.tets.size() << " " << mesh.tets.size() * 5 << "\n";
    for (auto& t : mesh.tets)
        out << "4 " << t.vi[0] << " " << t.vi[1] << " " << t.vi[2] << " " << t.vi[3] << "\n";
    out << "CELL_TYPES " << mesh.tets.size() << "\n";
    for (std::size_t i = 0; i < mesh.tets.size(); ++i)
        out << "10\n";  // VTK_TETRA
}
