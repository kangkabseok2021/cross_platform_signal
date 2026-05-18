#pragma once
#include <string>

// Logs one row per processing run to signal_runs table.
// Schema: run_id, ts, source_type, filter_type, sample_count,
//         fundamental_hz, rms_raw, rms_filtered, latency_us
class SqliteLogger {
public:
    explicit SqliteLogger(const std::string& db_path);
    ~SqliteLogger();

    struct RunRecord {
        std::string source_type;
        std::string filter_type;
        size_t      sample_count;
        double      fundamental_hz;
        double      rms_raw;
        double      rms_filtered;
        long long   latency_us;
    };

    void log(const RunRecord& rec);

private:
    void* db_{nullptr};   // sqlite3* — opaque to avoid header dependency
    void initSchema();
};
