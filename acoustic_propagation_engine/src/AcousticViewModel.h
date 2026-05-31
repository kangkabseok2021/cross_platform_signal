#pragma once
#include <QObject>
#include <QVariantList>
#include <QString>
#include <memory>
#include "AcousticEngine.h"

class AcousticViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList splGrid    READ splGrid    NOTIFY splGridChanged)
    Q_PROPERTY(double       sourceX    READ sourceX    NOTIFY sourceChanged)
    Q_PROPERTY(double       sourceY    READ sourceY    NOTIFY sourceChanged)
    Q_PROPERTY(double       sourceLw   READ sourceLw   WRITE setSourceLw   NOTIFY sourceLwChanged)
    Q_PROPERTY(QString      modelName  READ modelName  NOTIFY modelNameChanged)

public:
    explicit AcousticViewModel(QObject* parent = nullptr);

    QVariantList splGrid()   const;
    double       sourceX()   const { return m_srcX; }
    double       sourceY()   const { return m_srcY; }
    double       sourceLw()  const { return static_cast<double>(m_Lw); }
    QString      modelName() const;

    Q_INVOKABLE void setSource(double x, double y);
    Q_INVOKABLE void setSourceLw(double lw);
    Q_INVOKABLE void setModel(const QString& name);

signals:
    void splGridChanged();
    void sourceChanged();
    void sourceLwChanged();
    void modelNameChanged();

private:
    void recompute();

    AcousticEngine m_engine;
    float          m_srcX  = 25.0f;
    float          m_srcY  = 25.0f;
    float          m_Lw    = 80.0f;
    QVariantList   m_grid;
};
