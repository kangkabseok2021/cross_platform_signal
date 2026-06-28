#include "LaplacianSmoother.h"
#include <algorithm>

Mesh2D laplacian_smooth_2d(Mesh2D mesh, const std::vector<bool>& is_boundary, int n_iter) {
    int N = (int)mesh.nodes.size();
    // Build adjacency list from triangle connectivity
    std::vector<std::vector<int>> adj(N);
    for (auto& t : mesh.tris) {
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                if (i != j) adj[t.vi[i]].push_back(t.vi[j]);
    }
    for (auto& v : adj) {
        std::sort(v.begin(), v.end());
        v.erase(std::unique(v.begin(), v.end()), v.end());
    }

    for (int iter = 0; iter < n_iter; ++iter) {
        std::vector<Point2D> new_nodes = mesh.nodes;
        for (int i = 0; i < N; ++i) {
            if (is_boundary[i] || adj[i].empty()) continue;
            double sx = 0, sy = 0;
            for (int j : adj[i]) { sx += mesh.nodes[j].x; sy += mesh.nodes[j].y; }
            double cx = sx / adj[i].size();
            double cy = sy / adj[i].size();
            new_nodes[i] = Point2D{cx, cy};
        }
        mesh.nodes = new_nodes;
    }
    return mesh;
}

Mesh2D laplacian_smooth_2d(Mesh2D mesh, int boundary_count, int n_iter) {
    std::vector<bool> is_boundary(mesh.nodes.size(), false);
    for (int i = 0; i < boundary_count && i < (int)mesh.nodes.size(); ++i)
        is_boundary[i] = true;
    return laplacian_smooth_2d(std::move(mesh), is_boundary, n_iter);
}
