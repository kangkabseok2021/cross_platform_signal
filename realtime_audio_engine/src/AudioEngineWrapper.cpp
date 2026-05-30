#include "AudioEngineWrapper.h"
#include <QAbstractListModel>
#include <algorithm>

AudioEngineWrapper::AudioEngineWrapper(const QString& db_path, QObject* parent)
    : QObject(parent)
    , repo_(QStringLiteral("audio_main"), db_path)
    , preset_model_(&repo_, this)
{
    repo_.init();
    preset_model_.refresh();
    connect(&preset_model_, &PresetModel::presetApplied,
            this,            &AudioEngineWrapper::setCutoffHz);

    poll_timer_.setInterval(100);
    connect(&poll_timer_, &QTimer::timeout, this, &AudioEngineWrapper::onPollTimer);
    poll_timer_.start();

    engine_.start();
}

float AudioEngineWrapper::cutoffHz() const noexcept {
    return cutoff_hz_;
}

int AudioEngineWrapper::overruns() const noexcept {
    return overruns_;
}

QVector<float> AudioEngineWrapper::waveformBuffer() const {
    const auto snap = engine_.waveformSnapshot();
    return QVector<float>(snap.begin(), snap.end());
}

QAbstractListModel* AudioEngineWrapper::presetModel() {
    return &preset_model_;
}

void AudioEngineWrapper::setCutoffHz(float hz) {
    hz = std::clamp(hz, audio::kMinCutoffHz, audio::kMaxCutoffHz);
    if (hz == cutoff_hz_) return;
    cutoff_hz_ = hz;
    engine_.setCutoff(hz);
    emit cutoffHzChanged();
}

void AudioEngineWrapper::savePreset(const QString& name) {
    repo_.save({.id = -1, .name = name, .cutoff_hz = cutoff_hz_});
    preset_model_.refresh();
}

void AudioEngineWrapper::loadPreset(int id) {
    const QList<Preset> all = repo_.loadAll();
    for (const auto& p : all) {
        if (p.id == id) {
            setCutoffHz(p.cutoff_hz);
            break;
        }
    }
}

void AudioEngineWrapper::onPollTimer() {
    const auto diag = engine_.diagnostics();
    const int  ov   = static_cast<int>(std::min<uint64_t>(diag.overruns, INT_MAX));
    if (ov != overruns_) {
        overruns_ = ov;
        emit overrunsChanged();
    }
    emit waveformUpdated();
}
