#pragma once
#include <QObject>
#include <QTimer>
#include <QVector>
#include <QtQml/qqmlregistration.h>
#include "AudioEngine.h"
#include "PresetRepository.h"
#include "PresetModel.h"

class AudioEngineWrapper : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(float cutoffHz   READ cutoffHz   WRITE setCutoffHz NOTIFY cutoffHzChanged)
    Q_PROPERTY(int   overruns   READ overruns   NOTIFY overrunsChanged)
    Q_PROPERTY(QVector<float> waveformBuffer READ waveformBuffer NOTIFY waveformUpdated)
    Q_PROPERTY(QAbstractListModel* presetModel READ presetModel CONSTANT)

public:
    // db_path: pass ":memory:" for tests; empty uses the app-data directory.
    explicit AudioEngineWrapper(const QString& db_path = {},
                                QObject*       parent  = nullptr);
    ~AudioEngineWrapper() override = default;

    float  cutoffHz() const noexcept;
    int    overruns() const noexcept;
    QVector<float> waveformBuffer() const;
    QAbstractListModel* presetModel();

    Q_INVOKABLE void savePreset(const QString& name);
    Q_INVOKABLE void loadPreset(int id);

public slots:
    void setCutoffHz(float hz);

signals:
    void cutoffHzChanged();
    void overrunsChanged();
    void waveformUpdated();

private slots:
    void onPollTimer();

private:
    AudioEngine       engine_;
    PresetRepository  repo_;
    PresetModel       preset_model_;
    QTimer            poll_timer_;
    float             cutoff_hz_{audio::kDefaultCutoffHz};
    int               overruns_{0};
};
