#include "SqliteLogger.h"
#include <chrono>
#include <ctime>
#include "sqlite3.h"
#include <stdexcept>

SqliteLogger::SqliteLogger(const std::string& db_path) {
    if (sqlite3_open(db_path.c_str(), reinterpret_cast<sqlite3**>(&db_)) != SQLITE_OK)
        throw std::runtime_error("Cannot open DB: " + db_path);
    initSchema();
}

SqliteLogger::~SqliteLogger() {
    if (db_) sqlite3_close(reinterpret_cast<sqlite3*>(db_));
}

void SqliteLogger::initSchema() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS signal_runs (
            run_id        INTEGER PRIMARY KEY AUTOINCREMENT,
            ts            TEXT    NOT NULL,
            source_type   TEXT    NOT NULL,
            filter_type   TEXT    NOT NULL,
            sample_count  INTEGER NOT NULL,
            fundamental_hz REAL,
            rms_raw       REAL,
            rms_filtered  REAL,
            latency_us    INTEGER
        );
    )";
    char* err = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(db_), sql, nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
}

void SqliteLogger::log(const RunRecord& rec) {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    char ts[32]; std::strftime(ts, sizeof(ts), "%Y-%m-%dT%H:%M:%S", std::gmtime(&t));

    sqlite3_stmt* stmt = nullptr;
    auto* db = reinterpret_cast<sqlite3*>(db_);
    sqlite3_prepare_v2(db, R"(
        INSERT INTO signal_runs
            (ts, source_type, filter_type, sample_count,
             fundamental_hz, rms_raw, rms_filtered, latency_us)
        VALUES (?,?,?,?,?,?,?,?)
    )", -1, &stmt, nullptr);

    sqlite3_bind_text  (stmt, 1, ts,                          -1, SQLITE_TRANSIENT);
    sqlite3_bind_text  (stmt, 2, rec.source_type.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text  (stmt, 3, rec.filter_type.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64 (stmt, 4, static_cast<int64_t>(rec.sample_count));
    sqlite3_bind_double(stmt, 5, rec.fundamental_hz);
    sqlite3_bind_double(stmt, 6, rec.rms_raw);
    sqlite3_bind_double(stmt, 7, rec.rms_filtered);
    sqlite3_bind_int64 (stmt, 8, rec.latency_us);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr);
}
