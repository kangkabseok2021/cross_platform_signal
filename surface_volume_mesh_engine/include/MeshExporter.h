#pragma once
#include "Geometry3D.h"
#include <filesystem>

// Exports Mesh3D to VTK legacy unstructured grid format.
// Full implementation in Task 7; stub writes minimal valid header.
void export_vtk(const Mesh3D& mesh, const std::filesystem::path& path);
