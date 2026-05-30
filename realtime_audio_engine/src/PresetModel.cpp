#include "PresetModel.h"

PresetModel::PresetModel(PresetRepository* repo, QObject* parent)
    : QAbstractListModel(parent), repo_(repo)
{}

int PresetModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(presets_.size());
}

QVariant PresetModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= presets_.size()) return {};
    const auto& p = presets_.at(index.row());
    switch (role) {
    case IdRole:     return p.id;
    case NameRole:   return p.name;
    case CutoffRole: return static_cast<double>(p.cutoff_hz);
    default:         return {};
    }
}

QHash<int, QByteArray> PresetModel::roleNames() const {
    return {
        {IdRole,     "id"},
        {NameRole,   "name"},
        {CutoffRole, "cutoff"},
    };
}

void PresetModel::refresh() {
    beginResetModel();
    presets_ = repo_->loadAll();
    endResetModel();
}

void PresetModel::addPreset(const QString& name, float cutoff) {
    if (repo_->save({.id = -1, .name = name, .cutoff_hz = cutoff})) {
        refresh();
    }
}

void PresetModel::removePreset(int index) {
    if (index < 0 || index >= presets_.size()) return;
    const int id = presets_.at(index).id;
    if (repo_->remove(id)) {
        beginRemoveRows({}, index, index);
        presets_.removeAt(index);
        endRemoveRows();
    }
}

void PresetModel::applyPreset(int index) {
    if (index < 0 || index >= presets_.size()) return;
    emit presetApplied(presets_.at(index).cutoff_hz);
}
