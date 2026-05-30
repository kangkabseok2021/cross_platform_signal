#pragma once
#include <QAbstractListModel>
#include "PresetRepository.h"

class PresetModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles { IdRole = Qt::UserRole + 1, NameRole, CutoffRole };

    explicit PresetModel(PresetRepository* repo, QObject* parent = nullptr);

    int     rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void refresh();

signals:
    void presetApplied(float cutoff_hz);

public slots:
    void addPreset(const QString& name, float cutoff);
    void removePreset(int index);
    void applyPreset(int index);

private:
    PresetRepository* repo_;
    QList<Preset>     presets_;
};
