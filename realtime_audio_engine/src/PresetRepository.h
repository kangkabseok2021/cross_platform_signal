#pragma once
#include <QList>
#include <QString>

struct Preset {
    int     id{-1};
    QString name;
    float   cutoff_hz{1000.0f};
};

class PresetRepository {
public:
    // conn_name must be unique per QSqlDatabase connection (use ":memory:" for tests).
    explicit PresetRepository(const QString& conn_name = QStringLiteral("audio_main"),
                              const QString& db_path   = {});
    ~PresetRepository();

    bool init();
    [[nodiscard]] QList<Preset> loadAll() const;
    bool save(const Preset& p);
    bool remove(int id);

    [[nodiscard]] QString lastError() const { return last_error_; }

private:
    QString conn_name_;
    QString db_path_;
    mutable QString last_error_;
};
