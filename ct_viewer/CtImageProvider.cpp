#include "CtImageProvider.h"
#include <algorithm>
#include <cmath>

CtImageProvider::CtImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image) {}

QImage CtImageProvider::requestImage(const QString& /*id*/, QSize* size,
                                      const QSize& /*requestedSize*/) {
    QMutexLocker lock(&m_mutex);
    if (size) *size = m_current.size();
    return m_current;
}

void CtImageProvider::updateImage(const ct::Image& img) {
    if (img.data.empty()) return;

    // Normalise float pixels to 8-bit grayscale
    float mn = *std::min_element(img.data.begin(), img.data.end());
    float mx = *std::max_element(img.data.begin(), img.data.end());
    float range = (mx - mn) > 1e-6f ? (mx - mn) : 1.f;

    QImage qimg(static_cast<int>(img.size), static_cast<int>(img.size),
                QImage::Format_Grayscale8);
    for (std::size_t y = 0; y < img.size; ++y) {
        auto* line = qimg.scanLine(static_cast<int>(y));
        for (std::size_t x = 0; x < img.size; ++x) {
            float norm = (img.at(x, y) - mn) / range;
            line[x] = static_cast<uchar>(std::clamp(norm * 255.f, 0.f, 255.f));
        }
    }

    QMutexLocker lock(&m_mutex);
    m_current = qimg;
}
