#pragma once
#include <array>
#include <cmath>
#include <vector>

struct Point2D { double x, y; };
struct Triangle2D { std::array<int, 3> vi; };

struct Mesh2D {
    std::vector<Point2D>   nodes;
    std::vector<Triangle2D> tris;
};

// Positive if a,b,c are counterclockwise
inline double signed_area(const Point2D& a, const Point2D& b, const Point2D& c) {
    return 0.5 * ((b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y));
}

// Shewchuk exact in-circle predicate (CCW triangle assumed)
inline bool circumcircle_contains(const Point2D& a, const Point2D& b,
                                   const Point2D& c, const Point2D& p) {
    double ax = a.x - p.x, ay = a.y - p.y;
    double bx = b.x - p.x, by = b.y - p.y;
    double cx = c.x - p.x, cy = c.y - p.y;
    double det = ax * (by * (cx*cx + cy*cy) - cy * (bx*bx + by*by))
               - ay * (bx * (cx*cx + cy*cy) - cx * (bx*bx + by*by))
               + (ax*ax + ay*ay) * (bx*cy - by*cx);
    return det > 0.0;
}
