#include "heatmap_renderer.h"
#include <QColor>
#include <algorithm>
#include <cmath>

QImage HeatmapRenderer::toQImage(const std::vector<double>& T,
                                  int nx, int ny,
                                  double t_min, double t_max,
                                  int alpha)
{
    QImage img(nx, ny, QImage::Format_ARGB32);
    const double range = (t_max > t_min) ? (t_max - t_min) : 1.0;

    for (int j = 0; j < ny; ++j) {
        for (int i = 0; i < nx; ++i) {
            double norm = std::clamp((T[static_cast<size_t>(j*nx+i)] - t_min) / range,
                                     0.0, 1.0);
            // Hue: 0.667 (blue) → 0.333 (green) → 0.0 (red)
            double hue = 0.667 * (1.0 - norm);
            QColor c = QColor::fromHsvF(hue, 1.0, 1.0, alpha / 255.0);
            img.setPixel(i, j, c.rgba());
        }
    }
    return img;
}
