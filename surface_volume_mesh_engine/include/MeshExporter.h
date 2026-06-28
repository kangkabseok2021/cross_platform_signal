#pragma once
#include "Geometry2D.h"
#include "Geometry3D.h"
#include <filesystem>

void export_obj(const Mesh2D& mesh, const std::filesystem::path& path);
void export_vtk(const Mesh3D& mesh, const std::filesystem::path& path);
