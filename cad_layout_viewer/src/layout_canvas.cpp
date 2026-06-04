#include "layout_canvas.h"
#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>
#include <cmath>

const QList<QColor> LayoutCanvas::kLayerColors = {
    {  31, 119, 180}, {255, 127,  14}, { 44, 160,  44},
    {214,  39,  40}, {148, 103, 189}, {140,  86,  75},
    {227, 119, 194}, {127, 127, 127}, {188, 189,  34}, { 23, 190, 207}
};

LayoutCanvas::LayoutCanvas(QWidget* parent) : QOpenGLWidget(parent) {}

void LayoutCanvas::initializeGL() {}

void LayoutCanvas::resizeGL(int /*w*/, int /*h*/) {}

void LayoutCanvas::setLayout(const LayoutEngine* engine) {
    engine_ = engine;
    fitAll();
    update();
}

void LayoutCanvas::setHeatmap(const QImage& img) {
    heatmap_     = img;
    has_heatmap_ = !img.isNull();
    update();
}

void LayoutCanvas::clearHeatmap() {
    has_heatmap_ = false;
    update();
}

void LayoutCanvas::setLayerVisible(uint32_t layer_id, bool visible) {
    if (visible) hidden_layers_.erase(layer_id);
    else         hidden_layers_.insert(layer_id);
    update();
}

void LayoutCanvas::fitAll() {
    if (!engine_ || engine_->isEmpty()) return;
    QRectF bb = engine_->bbox();
    if (bb.isEmpty()) return;
    double sx = width()  / bb.width();
    double sy = height() / bb.height();
    scale_ = std::min(sx, sy) * 0.9;
    pan_   = QPointF(-bb.left() * scale_ + (width()  - bb.width()  * scale_) / 2.0,
                     -bb.top()  * scale_ + (height() - bb.height() * scale_) / 2.0);
    update();
}

void LayoutCanvas::paintGL() {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);
    p.fillRect(rect(), Qt::black);

    if (!engine_ || engine_->isEmpty()) return;

    p.save();
    p.translate(pan_);
    p.scale(scale_, scale_);

    int color_idx = 0;
    for (auto& [layer_id, polys] : engine_->layers()) {
        if (hidden_layers_.count(layer_id)) { ++color_idx; continue; }
        QColor c = kLayerColors[color_idx % kLayerColors.size()];
        p.setPen(QPen(c, 0));
        p.setBrush(c.lighter(150));
        for (const auto& poly : polys)
            p.drawPolygon(poly);
        ++color_idx;
    }

    if (has_heatmap_ && !heatmap_.isNull()) {
        QRectF bb = engine_->bbox();
        p.setOpacity(0.6);
        p.drawImage(bb, heatmap_);
        p.setOpacity(1.0);
    }
    p.restore();
}

void LayoutCanvas::wheelEvent(QWheelEvent* e) {
    double delta   = e->angleDelta().y() / 120.0;
    double factor  = std::pow(1.15, delta);
    QPointF mpos   = e->position();
    QPointF before = toScene(mpos);
    scale_ *= factor;
    QPointF after  = toScene(mpos);
    pan_ += (after - before) * scale_;
    update();
}

void LayoutCanvas::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::MiddleButton) {
        panning_    = true;
        last_mouse_ = e->position();
    }
}

void LayoutCanvas::mouseMoveEvent(QMouseEvent* e) {
    if (panning_) {
        QPointF d = e->position() - last_mouse_;
        pan_      += d;
        last_mouse_ = e->position();
        update();
    }
}

QPointF LayoutCanvas::toScene(const QPointF& screen) const {
    return QPointF((screen.x() - pan_.x()) / scale_,
                   (screen.y() - pan_.y()) / scale_);
}
