#include "layout_engine.h"
#include <QPointF>
#include <stdexcept>
#include <string>

void LayoutEngine::load(const QString& path)
{
    clear();
    std::unique_ptr<ParseResult, ParseResultDeleter> res{
        parse_layout(path.toUtf8().constData())};

    if (!res) throw std::runtime_error("parse_layout returned null");
    if (res->error_code != 0) {
        std::string msg = res->error_msg ? res->error_msg : "unknown parse error";
        throw std::runtime_error(msg);
    }

    double xmin = res->bbox_min_x, ymin = res->bbox_min_y;
    double xmax = res->bbox_max_x, ymax = res->bbox_max_y;
    bbox_ = QRectF(xmin, ymin, xmax - xmin, ymax - ymin);

    for (size_t i = 0; i < res->n_polygons; ++i) {
        const LayoutPolygon& lp = res->polygons[i];
        QPolygonF poly;
        poly.reserve(static_cast<qsizetype>(lp.n_points));
        for (size_t j = 0; j < lp.n_points; ++j)
            poly.append(QPointF(lp.x_coords[j], lp.y_coords[j]));
        layer_map_[lp.layer_id].push_back(std::move(poly));
    }
}

void LayoutEngine::clear() noexcept
{
    layer_map_.clear();
    bbox_ = {};
}

std::vector<uint32_t> LayoutEngine::layerIds() const
{
    std::vector<uint32_t> ids;
    ids.reserve(layer_map_.size());
    for (const auto& [id, _] : layer_map_) ids.push_back(id);
    return ids;
}
