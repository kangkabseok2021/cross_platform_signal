#include "SurveyGridGeometry.h"
#include "moc_SurveyGridGeometry.cpp"
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <QByteArray>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <vector>

SurveyGridGeometry::SurveyGridGeometry(Qt3DCore::QNode* parent)
    : Qt3DCore::QGeometry(parent)
{
    m_posBuf   = new Qt3DCore::QBuffer(this);
    m_colorBuf = new Qt3DCore::QBuffer(this);
    m_indexBuf = new Qt3DCore::QBuffer(this);

    m_posAttr = new Qt3DCore::QAttribute(this);
    m_posAttr->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
    m_posAttr->setVertexBaseType(Qt3DCore::QAttribute::Float);
    m_posAttr->setVertexSize(3);
    m_posAttr->setName(Qt3DCore::QAttribute::defaultPositionAttributeName());
    m_posAttr->setBuffer(m_posBuf);

    m_colorAttr = new Qt3DCore::QAttribute(this);
    m_colorAttr->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
    m_colorAttr->setVertexBaseType(Qt3DCore::QAttribute::Float);
    m_colorAttr->setVertexSize(3);
    m_colorAttr->setName(Qt3DCore::QAttribute::defaultColorAttributeName());
    m_colorAttr->setBuffer(m_colorBuf);

    m_indexAttr = new Qt3DCore::QAttribute(this);
    m_indexAttr->setAttributeType(Qt3DCore::QAttribute::IndexAttribute);
    m_indexAttr->setVertexBaseType(Qt3DCore::QAttribute::UnsignedInt);
    m_indexAttr->setBuffer(m_indexBuf);

    addAttribute(m_posAttr);
    addAttribute(m_colorAttr);
    addAttribute(m_indexAttr);
}

std::array<float,3> SurveyGridGeometry::jetColor(float t) {
    t = std::max(0.0f, std::min(1.0f, t));
    float r = std::max(0.0f, std::min(1.0f, 1.5f - std::abs(4.0f*t - 3.0f)));
    float g = std::max(0.0f, std::min(1.0f, 1.5f - std::abs(4.0f*t - 2.0f)));
    float b = std::max(0.0f, std::min(1.0f, 1.5f - std::abs(4.0f*t - 1.0f)));
    return {r, g, b};
}

void SurveyGridGeometry::updateGrid(const float* amplitudes, int w, int h) {
    if (w != m_w || h != m_h) rebuild(w, h);
    if (w <= 0 || h <= 0) return;

    float mn = amplitudes[0], mx = amplitudes[0];
    for (int i = 1; i < w*h; ++i) {
        mn = std::min(mn, amplitudes[i]);
        mx = std::max(mx, amplitudes[i]);
    }
    float range = (mx - mn > 1e-6f) ? (mx - mn) : 1.0f;

    QByteArray posData(w * h * 3 * static_cast<int>(sizeof(float)), Qt::Uninitialized);
    QByteArray colData(w * h * 3 * static_cast<int>(sizeof(float)), Qt::Uninitialized);
    auto* pos = reinterpret_cast<float*>(posData.data());
    auto* col = reinterpret_cast<float*>(colData.data());

    for (int r = 0; r < h; ++r) {
        for (int c = 0; c < w; ++c) {
            int vi = r*w + c;
            pos[vi*3+0] = static_cast<float>(c);
            pos[vi*3+1] = amplitudes[vi] / 100.0f;
            pos[vi*3+2] = static_cast<float>(r);
            auto rgb = jetColor((amplitudes[vi] - mn) / range);
            col[vi*3+0] = rgb[0]; col[vi*3+1] = rgb[1]; col[vi*3+2] = rgb[2];
        }
    }
    m_posBuf->setData(posData);
    m_colorBuf->setData(colData);
    m_posAttr->setCount(static_cast<unsigned int>(w * h));
    m_colorAttr->setCount(static_cast<unsigned int>(w * h));
}

void SurveyGridGeometry::rebuild(int w, int h) {
    m_w = w; m_h = h;
    if (w <= 1 || h <= 1) return;

    std::vector<unsigned int> indices;
    indices.reserve(static_cast<std::size_t>((w-1) * (h-1) * 6));
    for (int r = 0; r < h-1; ++r) {
        for (int c = 0; c < w-1; ++c) {
            unsigned int i0 = static_cast<unsigned int>(r*w + c);
            unsigned int i1 = i0 + 1;
            unsigned int i2 = i0 + static_cast<unsigned int>(w);
            unsigned int i3 = i2 + 1;
            indices.insert(indices.end(), {i0,i2,i1, i1,i2,i3});
        }
    }
    QByteArray idxData(static_cast<int>(indices.size() * sizeof(unsigned int)), Qt::Uninitialized);
    std::memcpy(idxData.data(), indices.data(), static_cast<std::size_t>(idxData.size()));
    m_indexBuf->setData(idxData);
    m_indexAttr->setCount(static_cast<unsigned int>(indices.size()));
}
