#pragma once
#include <Qt3DCore/QGeometry>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <array>
#include <vector>

class SurveyGridGeometry : public Qt3DCore::QGeometry {
    Q_OBJECT
public:
    explicit SurveyGridGeometry(Qt3DCore::QNode* parent = nullptr);
    void updateGrid(const float* amplitudes, int w, int h);
private:
    void rebuild(int w, int h);
    static std::array<float,3> jetColor(float t);

    Qt3DCore::QAttribute* m_posAttr   = nullptr;
    Qt3DCore::QAttribute* m_colorAttr = nullptr;
    Qt3DCore::QAttribute* m_indexAttr = nullptr;
    Qt3DCore::QBuffer*    m_posBuf    = nullptr;
    Qt3DCore::QBuffer*    m_colorBuf  = nullptr;
    Qt3DCore::QBuffer*    m_indexBuf  = nullptr;
    int m_w = 0, m_h = 0;
};
