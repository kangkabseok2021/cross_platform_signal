#pragma once
#include <QObject>
#include <QString>
#include <Qt3DCore/QEntity>
#include <memory>
#include <thread>
#include "Types.h"

class SurveyRepository;
class SurveyGridGeometry;

class SurveyController : public QObject {
    Q_OBJECT
    Q_PROPERTY(int depthIndex READ depthIndex WRITE setDepthIndex NOTIFY depthIndexChanged)
    Q_PROPERTY(float thresholdNt READ thresholdNt WRITE setThresholdNt NOTIFY thresholdNtChanged)
    Q_PROPERTY(bool processing READ processing NOTIFY processingChanged)
    Q_PROPERTY(int anomalyCount READ anomalyCount NOTIFY anomalyCountChanged)
    Q_PROPERTY(float progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(bool showAnomalies READ showAnomalies WRITE setShowAnomalies NOTIFY showAnomaliesChanged)
public:
    explicit SurveyController(QObject* parent = nullptr);
    ~SurveyController();

    int   depthIndex()    const { return m_depthIndex; }
    float thresholdNt()   const { return m_thresholdNt; }
    bool  processing()    const { return m_processing; }
    int   anomalyCount()  const { return m_anomalyCount; }
    float progress()      const { return m_progress; }
    bool  showAnomalies() const { return m_showAnomalies; }

    void setDepthIndex(int idx);
    void setThresholdNt(float t)  { m_thresholdNt = t;  emit thresholdNtChanged(); }
    void setShowAnomalies(bool s);

    void setSceneRoot(Qt3DCore::QEntity* root)    { m_sceneRoot = root; }
    void setGridGeometry(SurveyGridGeometry* geo) { m_gridGeometry = geo; }

    Q_INVOKABLE void startSurvey(const QString& name);
    Q_INVOKABLE void stopSurvey();
    Q_INVOKABLE void exportResults(const QString& format, const QString& path);
    Q_INVOKABLE void loadSession(int id);

signals:
    void depthIndexChanged();
    void thresholdNtChanged();
    void processingChanged();
    void anomalyCountChanged();
    void progressChanged();
    void showAnomaliesChanged();

public slots:
    void onProcessingComplete();

private:
    void clearAnomalyEntities();
    void buildAnomalyEntities(const std::vector<AnomalyCandidate>& candidates);

    int   m_depthIndex    = 0;
    float m_thresholdNt   = 50.0f;
    bool  m_processing    = false;
    int   m_anomalyCount  = 0;
    float m_progress      = 0.0f;
    bool  m_showAnomalies = true;

    GridBuffer m_gridBuffer;
    std::vector<AnomalyCandidate> m_candidates;
    int m_gridW = 20, m_gridH = 20;
    float m_cellM = 1.0f;

    Qt3DCore::QEntity*  m_sceneRoot    = nullptr;
    SurveyGridGeometry* m_gridGeometry = nullptr;
    QList<Qt3DCore::QEntity*> m_anomalyEntities;

    std::unique_ptr<SurveyRepository> m_repository;
    std::jthread m_producerThread;
    int m_currentSessionId = -1;
};
