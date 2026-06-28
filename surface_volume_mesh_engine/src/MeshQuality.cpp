#include "MeshQuality.h"
#include <cmath>
#include <limits>
#include <algorithm>
#include <numeric>

double aspect_ratio_2d(const Mesh2D& m, const Triangle2D& t) {
    const Point2D& a = m.nodes[t.vi[0]];
    const Point2D& b = m.nodes[t.vi[1]];
    const Point2D& c = m.nodes[t.vi[2]];
    double ea = std::hypot(b.x-c.x, b.y-c.y);
    double eb = std::hypot(a.x-c.x, a.y-c.y);
    double ec = std::hypot(a.x-b.x, a.y-b.y);
    double A = std::abs(signed_area(a, b, c));
    if (A < 1e-15) return std::numeric_limits<double>::infinity();
    double s = (ea + eb + ec) * 0.5;
    double R = ea * eb * ec / (4.0 * A);
    double r = A / s;
    return R / (2.0 * r);
}

double min_angle_deg_2d(const Mesh2D& m, const Triangle2D& t) {
    const Point2D& a = m.nodes[t.vi[0]];
    const Point2D& b = m.nodes[t.vi[1]];
    const Point2D& c = m.nodes[t.vi[2]];
    auto angle_at = [](const Point2D& p, const Point2D& q, const Point2D& r) {
        double ux=q.x-p.x, uy=q.y-p.y;
        double vx=r.x-p.x, vy=r.y-p.y;
        double dot = ux*vx + uy*vy;
        double cross = std::abs(ux*vy - uy*vx);
        return std::atan2(cross, dot) * 180.0 / M_PI;
    };
    return std::min({angle_at(a,b,c), angle_at(b,a,c), angle_at(c,a,b)});
}

double jacobian_det(const Mesh3D& m, const Tet& t) {
    return tet_volume(m.nodes[t.vi[0]], m.nodes[t.vi[1]],
                      m.nodes[t.vi[2]], m.nodes[t.vi[3]]) * 6.0;
}

double edge_ratio_3d(const Mesh3D& m, const Tet& t) {
    double edges[6]; int k = 0;
    for (int i = 0; i < 4; ++i)
        for (int j = i+1; j < 4; ++j) {
            const Point3D& a = m.nodes[t.vi[i]];
            const Point3D& b = m.nodes[t.vi[j]];
            edges[k++] = std::sqrt((b.x-a.x)*(b.x-a.x)+(b.y-a.y)*(b.y-a.y)+(b.z-a.z)*(b.z-a.z));
        }
    double mn = *std::min_element(edges, edges+6);
    double mx = *std::max_element(edges, edges+6);
    return mx > 0 ? mn / mx : 0.0;
}

QualityStats2D quality_report_2d(const Mesh2D& mesh) {
    if (mesh.tris.empty()) return {};
    QualityStats2D s;
    s.min_aspect_ratio = std::numeric_limits<double>::infinity();
    s.min_angle_deg    = 180.0;
    int below20 = 0;
    for (auto& t : mesh.tris) {
        double ar  = aspect_ratio_2d(mesh, t);
        double ang = min_angle_deg_2d(mesh, t);
        s.mean_aspect_ratio  += ar;
        s.mean_min_angle_deg += ang;
        s.min_aspect_ratio    = std::min(s.min_aspect_ratio, ar);
        s.min_angle_deg       = std::min(s.min_angle_deg, ang);
        if (ang < 20.0) ++below20;
    }
    double n = (double)mesh.tris.size();
    s.mean_aspect_ratio  /= n;
    s.mean_min_angle_deg /= n;
    s.frac_below_20deg    = below20 / n;
    return s;
}

QualityStats3D quality_report_3d(const Mesh3D& mesh) {
    if (mesh.tets.empty()) return {};
    QualityStats3D s;
    s.min_edge_ratio = 1.0;
    s.n_inverted     = mesh.n_inverted;
    for (auto& t : mesh.tets) {
        double er = edge_ratio_3d(mesh, t);
        s.mean_edge_ratio += er;
        s.min_edge_ratio   = std::min(s.min_edge_ratio, er);
    }
    s.mean_edge_ratio /= (double)mesh.tets.size();
    return s;
}
