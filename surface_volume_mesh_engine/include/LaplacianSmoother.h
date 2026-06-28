#pragma once
#include "Geometry2D.h"
#include <vector>

// Relax interior nodes to centroid of their neighbours.
// is_boundary[i] == true locks node i in place.
Mesh2D laplacian_smooth_2d(Mesh2D mesh, const std::vector<bool>& is_boundary, int n_iter);

// Convenience: mark all boundary polygon indices as locked.
// boundary_count = number of leading nodes in mesh.nodes that are boundary nodes.
Mesh2D laplacian_smooth_2d(Mesh2D mesh, int boundary_count, int n_iter);
