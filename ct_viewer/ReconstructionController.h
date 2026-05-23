#pragma once
#include <QObject>
#include <QString>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include "CtImageProvider.h"
#include "../ct_core/include/Types.h"
#include "../ct_core/include/FilterBank.h"
#include "../ct_core/include/Reconstructor.h"

class ReconstructionController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString filterType  READ filterType  WRITE setFilterType  NOTIFY filterTypeChanged)
    Q_PROPERTY(double  cutoffFreq  READ cutoffFreq  WRITE setCutoffFreq  NOTIFY cutoffFreqChanged)
    Q_PROPERTY(bool    isRunning   READ isRunning                        NOTIFY isRunningChanged)

public:
    explicit ReconstructionController(CtImageProvider* provider,
                                       QObject* parent = nullptr);

    [[nodiscard]] QString filterType() const { return m_filterType; }
    [[nodiscard]] double  cutoffFreq() const { return m_cutoffFreq; }
    [[nodiscard]] bool    isRunning()  const { return m_isRunning;  }

    void setFilterType(const QString& t);
    void setCutoffFreq(double f);

    Q_INVOKABLE void startReconstruction();

signals:
    void filterTypeChanged();
    void cutoffFreqChanged();
    void isRunningChanged();
    void reconstructionDone();

private:
    void runReconstruction();

    CtImageProvider* m_provider;
    QString          m_filterType{"RamLak"};
    double           m_cutoffFreq{1.0};
    bool             m_isRunning{false};
    QFuture<void>    m_future;
};
