#include "AdvancingFront2D.h"
#include <deque>
#include <cmath>
#include <limits>

int AdvancingFront2D::snap_node(const Point2D& ideal, const std::vector<Point2D>& nodes,
                                 int skip0, int skip1) const {
    double best = 0.6 * h_;
    int idx = -1;
    for (int k = 0; k < (int)nodes.size(); ++k) {
        if (k == skip0 || k == skip1) continue;
        double d = std::hypot(nodes[k].x - ideal.x, nodes[k].y - ideal.y);
        if (d < best) { best = d; idx = k; }
    }
    return idx;
}

Mesh2D AdvancingFront2D::mesh(const std::vector<Point2D>& boundary) const {
    Mesh2D m;
    m.nodes = boundary;

    // Domain centroid for inward-normal direction check
    Point2D cen{0.0, 0.0};
    for (auto& p : boundary) { cen.x += p.x; cen.y += p.y; }
    cen.x /= boundary.size(); cen.y /= boundary.size();

    // Front: pairs of node indices (CCW boundary edges)
    int nb = (int)boundary.size();
    std::deque<std::pair<int,int>> front;
    for (int i = 0; i < nb; ++i)
        front.push_back({i, (i + 1) % nb});

    while (!front.empty()) {
        auto [i0, i1] = front.front();
        front.pop_front();

        const Point2D& p0 = m.nodes[i0];
        const Point2D& p1 = m.nodes[i1];
        double len = std::hypot(p1.x - p0.x, p1.y - p0.y);
        if (len < 1e-12) continue;

        // Inward normal: 90 deg CCW rotation of (p0->p1) unit vector
        double nx = -(p1.y - p0.y) / len;
        double ny =  (p1.x - p0.x) / len;
        Point2D mid{(p0.x + p1.x) * 0.5, (p0.y + p1.y) * 0.5};
        // Flip if pointing away from centroid
        if ((cen.x - mid.x)*nx + (cen.y - mid.y)*ny < 0.0) { nx = -nx; ny = -ny; }

        Point2D ideal{mid.x + h_ * nx, mid.y + h_ * ny};

        int apex = snap_node(ideal, m.nodes, i0, i1);
        bool new_node = (apex == -1);
        if (new_node) {
            apex = (int)m.nodes.size();
            m.nodes.push_back(ideal);
        }

        // Ensure CCW orientation before inserting
        Triangle2D tri{i0, i1, apex};
        if (signed_area(m.nodes[tri.vi[0]], m.nodes[tri.vi[1]], m.nodes[tri.vi[2]]) < 0)
            std::swap(tri.vi[0], tri.vi[1]);
        m.tris.push_back(tri);

        if (new_node) {
            front.push_back({i1, apex});
            front.push_back({apex, i0});
        }
    }
    return m;
}
