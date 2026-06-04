#pragma once
#include <QImage>
#include <vector>

// Converts a temperature field into a semi-transparent QImage heatmap.
// Colormap: blue (cold) → green (mid) → red (hot)  via HSV interpolation.
class HeatmapRenderer {
public:
    // alpha: 0=transparent, 255=opaque  (default ≈63% opacity)
    static QImage toQImage(const std::vector<double>& T,
                           int nx, int ny,
                           double t_min, double t_max,
                           int alpha = 160);
};
