#pragma once
#include <QList>
#include <QPointF>
#include <vector>

struct HotspotInfo {
    QPointF centroid;
    double  max_temp{0.0};
    double  area_um2{0.0};   // approximate
    int     grid_x{0}, grid_y{0};  // grid-cell of maximum
};

class HotspotAnalyser {
public:
    // threshold_pct: fraction of T_max that defines "hot" (e.g. 0.9 = 90%)
    static QList<HotspotInfo> detect(const std::vector<double>& T,
                                     int nx, int ny,
                                     double t_max,
                                     double threshold_pct = 0.9,
                                     double cell_um = 1.0);
};
