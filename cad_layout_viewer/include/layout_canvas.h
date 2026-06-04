#pragma once
#include "layout_engine.h"
#include "heatmap_renderer.h"
#include <QImage>
#include <QWidget>
#include <QPointF>
#include <unordered_set>

// Uses QWidget + QPainter (Qt6 raster/OpenGL backend).
class LayoutCanvas : public QWidget {
    Q_OBJECT
public:
    explicit LayoutCanvas(QWidget* parent = nullptr);

    void setLayout(const LayoutEngine* engine);
    void setHeatmap(const QImage& img);
    void clearHeatmap();
    void setLayerVisible(uint32_t layer_id, bool visible);
    void fitAll();

signals:
    void polygonSelected(uint32_t layer_id, qsizetype polygon_idx);

protected:
    void paintEvent(QPaintEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;

private:
    QPointF toScene(const QPointF& screen) const;

    const LayoutEngine* engine_{nullptr};
    QImage              heatmap_;
    bool                has_heatmap_{false};
    std::unordered_set<uint32_t> hidden_layers_;

    // View state
    double  scale_{1.0};
    QPointF pan_{0, 0};
    QPointF last_mouse_;
    bool    panning_{false};

    static const QList<QColor> kLayerColors;
};
