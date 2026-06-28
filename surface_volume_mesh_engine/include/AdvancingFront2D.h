#pragma once
#include "Geometry2D.h"

class AdvancingFront2D {
public:
    explicit AdvancingFront2D(double h) : h_(h) {}
    // boundary: ordered CCW polygon vertices defining the domain boundary
    Mesh2D mesh(const std::vector<Point2D>& boundary) const;
private:
    double h_;
    // Returns index of nearest existing node within snap_radius, or -1
    int snap_node(const Point2D& ideal, const std::vector<Point2D>& nodes,
                  int skip0, int skip1) const;
};
