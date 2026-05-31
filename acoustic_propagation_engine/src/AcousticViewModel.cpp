#include "AcousticViewModel.h"
#include "InverseSquareLawModel.h"
#include "SabineReverbModel.h"
#include <cmath>

static constexpr int   kGridN    = 50;
static constexpr float kGridSize = 100.0f;   /* metres */

AcousticViewModel::AcousticViewModel(QObject* parent)
    : QObject(parent)
{
    recompute();
}

void AcousticViewModel::setSource(double x, double y)
{
    m_srcX = static_cast<float>(x);
    m_srcY = static_cast<float>(y);
    recompute();
    emit sourceChanged();
}

void AcousticViewModel::setSourceLw(double lw)
{
    m_Lw = static_cast<float>(lw);
    recompute();
    emit sourceLwChanged();
}

void AcousticViewModel::setModel(const QString& name)
{
    if (name == "SabineReverb")
        m_engine.setModel(std::make_unique<SabineReverbModel>(200.0f, 40.0f, 160.0f));
    else
        m_engine.setModel(std::make_unique<InverseSquareLawModel>());
    recompute();
    emit modelNameChanged();
}

QString AcousticViewModel::modelName() const
{
    return QString::fromUtf8(m_engine.currentModelName().data(),
                             static_cast<qsizetype>(m_engine.currentModelName().size()));
}

QVariantList AcousticViewModel::splGrid() const { return m_grid; }

void AcousticViewModel::recompute()
{
    std::vector<float> coords(kGridN);
    for (int i = 0; i < kGridN; ++i)
        coords[static_cast<size_t>(i)] = (i + 0.5f) * kGridSize / kGridN;

    const auto grid = m_engine.computeGrid(m_Lw, m_srcX, m_srcY, coords, coords);

    m_grid.clear();
    m_grid.reserve(static_cast<qsizetype>(grid.size()));
    for (float v : grid) m_grid.append(static_cast<double>(v));

    emit splGridChanged();
}
