#include "SurveyRepository.h"
#include "moc_SurveyRepository.cpp"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDateTime>
#include <QDebug>
#include <cmath>

SurveyRepository::SurveyRepository(QObject* parent) : QObject(parent) {}

bool SurveyRepository::open(const QString& dbPath) {
    m_db = QSqlDatabase::addDatabase("QSQLITE", "sensys_conn");
    m_db.setDatabaseName(dbPath);
    if (!m_db.open()) {
        qWarning() << "SurveyRepository: cannot open" << dbPath << m_db.lastError().text();
        return false;
    }
    QSqlQuery q(m_db);
    q.exec("PRAGMA journal_mode=WAL");
    return createSchema();
}

bool SurveyRepository::createSchema() {
    QSqlQuery q(m_db);
    bool ok = true;
    ok &= q.exec("CREATE TABLE IF NOT EXISTS survey_sessions("
                 "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                 "name TEXT, created_at TEXT,"
                 "grid_w INTEGER, grid_h INTEGER, cell_m REAL,"
                 "n_channels INTEGER, n_depths INTEGER, anomaly_count INTEGER DEFAULT 0)");
    ok &= q.exec("CREATE TABLE IF NOT EXISTS scan_points("
                 "id INTEGER PRIMARY KEY AUTOINCREMENT, session_id INTEGER,"
                 "x REAL, y REAL, z REAL, ch INTEGER, amplitude_nT REAL, raw_nT REAL)");
    ok &= q.exec("CREATE INDEX IF NOT EXISTS idx_sp ON scan_points(session_id, z, y, x)");
    ok &= q.exec("CREATE TABLE IF NOT EXISTS anomaly_candidates("
                 "id INTEGER PRIMARY KEY AUTOINCREMENT, session_id INTEGER,"
                 "grid_x INTEGER, grid_y INTEGER, depth_m REAL, peak_nT REAL, ch INTEGER)");
    if (!ok) qWarning() << "SurveyRepository schema error:" << q.lastError().text();
    return ok;
}

int SurveyRepository::saveSession(const QString& name, int w, int h, float cell_m,
                                   int n_ch, int n_depths,
                                   const std::vector<ScanPoint>& points,
                                   const std::vector<AnomalyCandidate>& candidates) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO survey_sessions(name,created_at,grid_w,grid_h,cell_m,n_channels,n_depths,anomaly_count)"
              " VALUES(?,?,?,?,?,?,?,?)");
    q.addBindValue(name);
    q.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    q.addBindValue(w); q.addBindValue(h); q.addBindValue(static_cast<double>(cell_m));
    q.addBindValue(n_ch); q.addBindValue(n_depths);
    q.addBindValue(static_cast<int>(candidates.size()));
    if (!q.exec()) { qWarning() << q.lastError().text(); return -1; }
    int session_id = q.lastInsertId().toInt();

    q.exec("BEGIN TRANSACTION");
    q.prepare("INSERT INTO scan_points(session_id,x,y,z,ch,amplitude_nT,raw_nT) VALUES(?,?,?,?,?,?,?)");
    int batch = 0;
    for (const auto& p : points) {
        q.addBindValue(session_id);
        q.addBindValue(static_cast<double>(p.x)); q.addBindValue(static_cast<double>(p.y));
        q.addBindValue(static_cast<double>(p.z)); q.addBindValue(p.channel);
        q.addBindValue(static_cast<double>(p.amplitude_nT));
        q.addBindValue(static_cast<double>(p.raw_nT));
        q.exec();
        if (++batch % 1000 == 0) { q.exec("COMMIT"); q.exec("BEGIN TRANSACTION"); }
    }
    q.exec("COMMIT");

    for (const auto& c : candidates) {
        q.prepare("INSERT INTO anomaly_candidates(session_id,grid_x,grid_y,depth_m,peak_nT,ch) VALUES(?,?,?,?,?,?)");
        q.addBindValue(session_id); q.addBindValue(c.grid_x); q.addBindValue(c.grid_y);
        q.addBindValue(static_cast<double>(c.depth_m)); q.addBindValue(static_cast<double>(c.peak_nT));
        q.addBindValue(c.channel); q.exec();
    }
    return session_id;
}

GridBuffer SurveyRepository::loadSession(int session_id, int w, int h, int n_depths) {
    GridBuffer buf;
    buf.width = w; buf.height = h; buf.n_depths = n_depths;
    buf.data.assign(static_cast<std::size_t>(w * h * n_depths), 0.0f);

    QSqlQuery q(m_db);
    q.prepare("SELECT x,y,z,amplitude_nT FROM scan_points WHERE session_id=? ORDER BY z,y,x");
    q.addBindValue(session_id);
    if (!q.exec()) return buf;

    std::vector<float> z_levels;
    while (q.next()) {
        float z = static_cast<float>(q.value(2).toDouble());
        bool found = false;
        for (float zl : z_levels) if (std::abs(zl - z) < 1e-4f) { found = true; break; }
        if (!found) z_levels.push_back(z);
    }

    q.prepare("SELECT x,y,z,amplitude_nT FROM scan_points WHERE session_id=? ORDER BY z,y,x");
    q.addBindValue(session_id);
    q.exec();
    while (q.next()) {
        float x = static_cast<float>(q.value(0).toDouble());
        float y = static_cast<float>(q.value(1).toDouble());
        float z = static_cast<float>(q.value(2).toDouble());
        float amp = static_cast<float>(q.value(3).toDouble());
        int col = static_cast<int>(std::round(x));
        int row = static_cast<int>(std::round(y));
        int depth = 0;
        for (int d = 0; d < static_cast<int>(z_levels.size()); ++d)
            if (std::abs(z_levels[d] - z) < 1e-4f) { depth = d; break; }
        if (col >= 0 && col < w && row >= 0 && row < h && depth < n_depths)
            buf.at(depth, row, col) = amp;
    }
    return buf;
}

QList<SessionMeta> SurveyRepository::listSessions() {
    QList<SessionMeta> list;
    QSqlQuery q("SELECT id,name,created_at,grid_w,grid_h,cell_m,n_channels,n_depths,anomaly_count"
                " FROM survey_sessions ORDER BY id DESC", m_db);
    while (q.next()) {
        SessionMeta m;
        m.id = q.value(0).toInt(); m.name = q.value(1).toString();
        m.created_at = q.value(2).toString();
        m.grid_w = q.value(3).toInt(); m.grid_h = q.value(4).toInt();
        m.cell_m = static_cast<float>(q.value(5).toDouble());
        m.n_channels = q.value(6).toInt(); m.n_depths = q.value(7).toInt();
        m.anomaly_count = q.value(8).toInt();
        list.append(m);
    }
    return list;
}
