#pragma once
#include "layout_ffi.h"
#include <QPolygonF>
#include <QRectF>
#include <QString>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

struct ParseResultDeleter {
    void operator()(ParseResult* p) const noexcept { free_parse_result(p); }
};

class LayoutEngine {
public:
    void load(const QString& path);
    void clear() noexcept;

    [[nodiscard]] bool    isEmpty() const noexcept { return layer_map_.empty(); }
    [[nodiscard]] QRectF  bbox()    const noexcept { return bbox_; }

    [[nodiscard]] const std::unordered_map<uint32_t, std::vector<QPolygonF>>&
        layers() const noexcept { return layer_map_; }

    [[nodiscard]] std::vector<uint32_t> layerIds() const;

private:
    std::unordered_map<uint32_t, std::vector<QPolygonF>> layer_map_;
    QRectF bbox_;
};
