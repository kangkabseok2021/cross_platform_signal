#include "DelaunayMesh3D.h"
#include <algorithm>
#include <map>
#include <random>

Mesh3D DelaunayMesh3D::triangulate(std::vector<Point3D> points) const {
    if (points.size() < 4) return {};

    // Bounding box for super-tetrahedron
    double big = 0;
    for (auto& p : points)
        big = std::max(big, std::max({std::abs(p.x), std::abs(p.y), std::abs(p.z)}));
    big = big * 5.0 + 10.0;

    Mesh3D m;
    // Super-tetrahedron vertices (indices 0-3)
    m.nodes = {
        { 0,       big*3,  0      },
        {-big*3,  -big,    0      },
        { big*3,  -big,    0      },
        { 0,       0,      big*3  }
    };
    m.tets = {{ {0, 1, 2, 3} }};

    // Randomise insertion order for O(n log n) expected
    std::mt19937 rng(42);
    std::shuffle(points.begin(), points.end(), rng);

    for (auto& pt : points) {
        int pidx = (int)m.nodes.size();
        m.nodes.push_back(pt);

        // Find cavity: all tets whose circumsphere contains pt
        std::vector<Tet> bad, good;
        for (auto& t : m.tets) {
            if (circumsphere_contains(m.nodes[t.vi[0]], m.nodes[t.vi[1]],
                                       m.nodes[t.vi[2]], m.nodes[t.vi[3]], pt))
                bad.push_back(t);
            else
                good.push_back(t);
        }

        // Boundary faces of cavity: faces shared by exactly one bad tet
        std::map<std::array<int,3>, int> face_cnt;
        for (auto& t : bad) {
            for (int i = 0; i < 4; ++i) {
                std::array<int,3> f;
                int k = 0;
                for (int j = 0; j < 4; ++j) if (j != i) f[k++] = t.vi[j];
                std::sort(f.begin(), f.end());
                face_cnt[f]++;
            }
        }

        m.tets = good;
        for (auto& [f, cnt] : face_cnt) {
            if (cnt != 1) continue;
            Tet nt{{ f[0], f[1], f[2], pidx }};
            // Ensure positive volume
            if (tet_volume(m.nodes[nt.vi[0]], m.nodes[nt.vi[1]],
                           m.nodes[nt.vi[2]], m.nodes[nt.vi[3]]) < 0)
                std::swap(nt.vi[0], nt.vi[1]);
            m.tets.push_back(nt);
        }
    }

    // Remove tets touching super-tet vertices (indices 0-3), re-index
    std::vector<Tet> final_tets;
    for (auto& t : m.tets) {
        bool touches = false;
        for (int v : t.vi) if (v < 4) { touches = true; break; }
        if (!touches) {
            Tet nt = t;
            for (int& v : nt.vi) v -= 4;  // shift: input points start at index 4
            final_tets.push_back(nt);
        }
    }
    // Replace nodes with just the input points (drop super-tet vertices)
    m.nodes = points;

    // Remove degenerate (zero-volume) tets produced from coplanar groups
    {
        std::vector<Tet> non_deg;
        non_deg.reserve(final_tets.size());
        for (auto& t : final_tets)
            if (tet_volume(m.nodes[t.vi[0]], m.nodes[t.vi[1]],
                           m.nodes[t.vi[2]], m.nodes[t.vi[3]]) > 0)
                non_deg.push_back(t);
        m.tets = std::move(non_deg);
    }

    // Count inverted (should be 0 after degenerate removal + positive-vol swap)
    m.n_inverted = 0;
    for (auto& t : m.tets)
        if (tet_volume(m.nodes[t.vi[0]], m.nodes[t.vi[1]],
                       m.nodes[t.vi[2]], m.nodes[t.vi[3]]) < 0)
            ++m.n_inverted;

    return m;
}
