#pragma once
#include <QObject>
#include <QList>
#include <QString>
#include <QtSql/QSqlDatabase>
#include "Types.h"

struct SessionMeta {
    int id = 0;
    QString name;
    QString created_at;
    int grid_w = 0, grid_h = 0;
    float cell_m = 0.0f;
    int n_channels = 0, n_depths = 0;
    int anomaly_count = 0;
};

class SurveyRepository : public QObject {
    Q_OBJECT
public:
    explicit SurveyRepository(QObject* parent = nullptr);
    bool open(const QString& dbPath);
    int saveSession(const QString& name, int w, int h, float cell_m, int n_ch, int n_depths,
                    const std::vector<ScanPoint>& points,
                    const std::vector<AnomalyCandidate>& candidates);
    GridBuffer loadSession(int session_id, int w, int h, int n_depths);
    QList<SessionMeta> listSessions();
private:
    bool createSchema();
    QSqlDatabase m_db;
};
