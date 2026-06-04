#include "hotspot_analyser.h"
#include <algorithm>
#include <queue>
#include <vector>

QList<HotspotInfo> HotspotAnalyser::detect(const std::vector<double>& T,
                                             int nx, int ny,
                                             double t_max,
                                             double threshold_pct,
                                             double cell_um)
{
    const double threshold = threshold_pct * t_max;
    std::vector<bool> visited(static_cast<size_t>(nx * ny), false);
    QList<HotspotInfo> results;

    auto idx = [&](int x, int y) { return y * nx + x; };

    for (int j = 0; j < ny; ++j) {
        for (int i = 0; i < nx; ++i) {
            if (visited[static_cast<size_t>(idx(i,j))] || T[static_cast<size_t>(idx(i,j))] < threshold)
                continue;

            // BFS flood fill
            std::queue<std::pair<int,int>> q;
            q.push({i, j});
            visited[static_cast<size_t>(idx(i,j))] = true;

            HotspotInfo hs;
            double sum_x = 0, sum_y = 0;
            int count = 0;
            double local_max = T[static_cast<size_t>(idx(i,j))];

            while (!q.empty()) {
                auto [cx, cy] = q.front(); q.pop();
                double t = T[static_cast<size_t>(idx(cx,cy))];
                sum_x += cx; sum_y += cy;
                ++count;
                if (t > local_max) {
                    local_max = t;
                    hs.grid_x = cx; hs.grid_y = cy;
                }
                for (auto [dx,dy] : std::initializer_list<std::pair<int,int>>{{1,0},{-1,0},{0,1},{0,-1}}) {
                    int nx2 = cx+dx, ny2 = cy+dy;
                    if (nx2 < 0 || nx2 >= nx || ny2 < 0 || ny2 >= ny) continue;
                    size_t ni = static_cast<size_t>(idx(nx2,ny2));
                    if (!visited[ni] && T[ni] >= threshold) {
                        visited[ni] = true;
                        q.push({nx2, ny2});
                    }
                }
            }
            hs.centroid  = QPointF(sum_x / count, sum_y / count);
            hs.max_temp  = local_max;
            hs.area_um2  = count * cell_um * cell_um;
            results.append(hs);
        }
    }
    return results;
}
