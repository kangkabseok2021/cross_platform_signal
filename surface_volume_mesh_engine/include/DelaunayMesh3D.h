#pragma once
#include "Geometry3D.h"

class DelaunayMesh3D {
public:
    // Returns Delaunay tetrahedralization of the input point set.
    // n_inverted counts tetrahedra with negative volume (should be 0 for clean inputs).
    Mesh3D triangulate(std::vector<Point3D> points) const;
};
