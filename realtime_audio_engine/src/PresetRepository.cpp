#include "PresetRepository.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>

PresetRepository::PresetRepository(const QString& conn_name, const QString& db_path)
    : conn_name_(conn_name)
    , db_path_(db_path)
{}

PresetRepository::~PresetRepository() {
    QSqlDatabase::removeDatabase(conn_name_);
}

bool PresetRepository::init() {
    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), conn_name_);
    if (db_path_.isEmpty()) {
        const QString dir = QStandardPaths::writableLocation(
                                QStandardPaths::AppDataLocation);
        QDir{}.mkpath(dir);
        db.setDatabaseName(dir + QStringLiteral("/audio_presets.db"));
    } else {
        db.setDatabaseName(db_path_);
    }

    if (!db.open()) {
        last_error_ = db.lastError().text();
        return false;
    }

    QSqlQuery q(db);
    q.exec(QStringLiteral("PRAGMA journal_mode=WAL"));
    q.exec(QStringLiteral("PRAGMA foreign_keys=ON"));

    if (!q.exec(QStringLiteral(
            "CREATE TABLE IF NOT EXISTS presets ("
            "  id         INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  name       TEXT    NOT NULL UNIQUE,"
            "  cutoff_hz  REAL    NOT NULL"
            "    CHECK(cutoff_hz BETWEEN 20 AND 20000),"
            "  created_at TEXT    DEFAULT (datetime('now'))"
            ")"))) {
        last_error_ = q.lastError().text();
        return false;
    }
    return true;
}

QList<Preset> PresetRepository::loadAll() const {
    QSqlDatabase db = QSqlDatabase::database(conn_name_);
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT id, name, cutoff_hz FROM presets ORDER BY name"));
    QList<Preset> result;
    if (!q.exec()) {
        last_error_ = q.lastError().text();
        return result;
    }
    while (q.next()) {
        result.append({q.value(0).toInt(),
                       q.value(1).toString(),
                       q.value(2).toFloat()});
    }
    return result;
}

bool PresetRepository::save(const Preset& p) {
    QSqlDatabase db = QSqlDatabase::database(conn_name_);
    QSqlQuery q(db);
    q.prepare(QStringLiteral(
        "INSERT INTO presets(name, cutoff_hz) VALUES(:name, :cutoff)"));
    q.bindValue(QStringLiteral(":name"),   p.name);
    q.bindValue(QStringLiteral(":cutoff"), static_cast<double>(p.cutoff_hz));
    if (!q.exec()) {
        last_error_ = q.lastError().text();
        return false;
    }
    return true;
}

bool PresetRepository::remove(int id) {
    QSqlDatabase db = QSqlDatabase::database(conn_name_);
    QSqlQuery q(db);
    q.prepare(QStringLiteral("DELETE FROM presets WHERE id = :id"));
    q.bindValue(QStringLiteral(":id"), id);
    if (!q.exec()) {
        last_error_ = q.lastError().text();
        return false;
    }
    return true;
}
