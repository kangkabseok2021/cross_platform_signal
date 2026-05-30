#include "TelemetryModel.h"

TelemetryModel::TelemetryModel(QObject* parent)
    : QObject(parent), filters_(0.1)
{
    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &TelemetryModel::onTick);
    timer_->start(100);
}

void TelemetryModel::onTick() {
    sim_.update();
    auto raw = sim_.rawValues();
    std::array<double, 4> filtered{};
    filters_.processAll(raw, filtered);

    std::lock_guard<std::mutex> lock(mutex_);
    const auto& simCh = sim_.channels();
    for (std::size_t i = 0; i < 4; ++i) {
        channels_[i]          = simCh[i];
        channels_[i].filtered = filtered[i];
    }
    emit dataChanged();
}

double TelemetryModel::engineTempRaw()       const { std::lock_guard<std::mutex> lk(mutex_); return channels_[0].raw; }
double TelemetryModel::engineTempFiltered()  const { std::lock_guard<std::mutex> lk(mutex_); return channels_[0].filtered; }
double TelemetryModel::batteryVRaw()         const { std::lock_guard<std::mutex> lk(mutex_); return channels_[1].raw; }
double TelemetryModel::batteryVFiltered()    const { std::lock_guard<std::mutex> lk(mutex_); return channels_[1].filtered; }
double TelemetryModel::oilPressureRaw()      const { std::lock_guard<std::mutex> lk(mutex_); return channels_[2].raw; }
double TelemetryModel::oilPressureFiltered() const { std::lock_guard<std::mutex> lk(mutex_); return channels_[2].filtered; }
double TelemetryModel::rpmRaw()              const { std::lock_guard<std::mutex> lk(mutex_); return channels_[3].raw; }
double TelemetryModel::rpmFiltered()         const { std::lock_guard<std::mutex> lk(mutex_); return channels_[3].filtered; }
bool   TelemetryModel::engineFault()         const { std::lock_guard<std::mutex> lk(mutex_); return channels_[0].fault; }
bool   TelemetryModel::batteryFault()        const { std::lock_guard<std::mutex> lk(mutex_); return channels_[1].fault; }

void TelemetryModel::setAlpha(int channelId, double alpha) {
    if (channelId >= 0 && channelId < 4)
        filters_.setAlpha(static_cast<std::size_t>(channelId), alpha);
}

void TelemetryModel::resetChannel(int channelId) {
    if (channelId >= 0 && channelId < 4)
        filters_.reset(static_cast<std::size_t>(channelId));
}

void TelemetryModel::injectFault(int channelId, double magnitude) {
    sim_.injectFault(channelId, magnitude);
}

TelemetrySnapshot TelemetryModel::telemetrySnapshot() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return {channels_, {filters_.alpha(0), filters_.alpha(1), filters_.alpha(2), filters_.alpha(3)}};
}
