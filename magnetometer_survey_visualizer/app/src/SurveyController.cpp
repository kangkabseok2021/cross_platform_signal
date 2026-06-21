#include "SurveyController.h"
#include "moc_SurveyController.cpp"
#include "SurveyRepository.h"
#include "SurveyGridGeometry.h"
#include "GradiometerSimulator.h"
#include "GaussianSmoother.h"
#include "BaselineSubtractor.h"
#include "AnomalyDetector.h"
#include "AnomalyExporter.h"
#include <QThreadPool>
#include <QRunnable>
#include <QReadWriteLock>
#include <QMetaObject>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DCore/QTransform>
#include <QStandardPaths>
#include <cmath>

struct ScanLineTask : public QRunnable {
    std::vector<float> line_data;
    int row, width;
    GridBuffer* buffer;
    QReadWriteLock* lock;

    void run() override {
        // GaussianSmoother needs vertical neighbours — applied to full grid
        // after all lines complete in onProcessingComplete().
        BaselineSubtractor::subtract(line_data.data(), width, 1);
        QWriteLocker lk(lock);
        for (int c = 0; c < width; ++c)
            buffer->at(0, row, c) = line_data[static_cast<std::size_t>(c)];
    }
};

SurveyController::SurveyController(QObject* parent) : QObject(parent) {
    m_repository = std::make_unique<SurveyRepository>(this);
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                     + "/surveys.db";
    m_repository->open(dbPath);
}

SurveyController::~SurveyController() {
    if (m_producerThread.joinable()) m_producerThread.request_stop();
}

void SurveyController::setDepthIndex(int idx) {
    m_depthIndex = idx;
    emit depthIndexChanged();
    if (m_gridGeometry && !m_gridBuffer.data.empty()) {
        int off = m_depthIndex * m_gridH * m_gridW;
        m_gridGeometry->updateGrid(m_gridBuffer.data.data() + off, m_gridW, m_gridH);
    }
}

void SurveyController::setShowAnomalies(bool s) {
    m_showAnomalies = s;
    for (auto* e : m_anomalyEntities) e->setEnabled(s);
    emit showAnomaliesChanged();
}

void SurveyController::startSurvey(const QString& name) {
    if (m_processing) return;
    m_processing = true;
    m_progress = 0.0f;
    emit processingChanged();
    emit progressChanged();

    clearAnomalyEntities();

    m_gridBuffer.width = m_gridW;
    m_gridBuffer.height = m_gridH;
    m_gridBuffer.n_depths = 1;
    m_gridBuffer.data.assign(static_cast<std::size_t>(m_gridW * m_gridH), 0.0f);

    QString sessionName = name;
    m_producerThread = std::jthread([this, sessionName](std::stop_token st) {
        GradiometerSimulator sim(m_gridW, m_gridH, m_cellM, 8, 42);
        sim.addDipole({static_cast<float>(m_gridW) * 0.5f * m_cellM,
                       static_cast<float>(m_gridH) * 0.5f * m_cellM,
                       0.5f, 100000.0f});
        auto pts = sim.generate();

        QReadWriteLock rwlock;
        int total = m_gridH;
        for (int row = 0; row < m_gridH && !st.stop_requested(); ++row) {
            auto* task = new ScanLineTask();
            task->row = row; task->width = m_gridW;
            task->buffer = &m_gridBuffer; task->lock = &rwlock;
            task->line_data.resize(static_cast<std::size_t>(m_gridW));

            for (const auto& p : pts) {
                if (p.channel == 0 && static_cast<int>(std::round(p.y / m_cellM)) == row) {
                    int col = static_cast<int>(std::round(p.x / m_cellM));
                    if (col >= 0 && col < m_gridW)
                        task->line_data[static_cast<std::size_t>(col)] = p.amplitude_nT;
                }
            }

            task->setAutoDelete(true);
            QThreadPool::globalInstance()->start(task);

            float prog = static_cast<float>(row + 1) / static_cast<float>(total);
            QMetaObject::invokeMethod(this, [this, prog]() {
                m_progress = prog; emit progressChanged();
            }, Qt::QueuedConnection);
        }

        QThreadPool::globalInstance()->waitForDone();
        QMetaObject::invokeMethod(this, "onProcessingComplete", Qt::QueuedConnection);
    });
}

void SurveyController::stopSurvey() {
    m_producerThread.request_stop();
}

void SurveyController::onProcessingComplete() {
    GaussianSmoother::apply(m_gridBuffer.data.data(), m_gridW, m_gridH);

    m_candidates = AnomalyDetector::find(
        m_gridBuffer.data.data(), m_gridW, m_gridH, m_thresholdNt);

    m_currentSessionId = m_repository->saveSession(
        "survey", m_gridW, m_gridH, m_cellM, 8, 1, {}, m_candidates);

    m_anomalyCount = static_cast<int>(m_candidates.size());
    emit anomalyCountChanged();

    if (m_gridGeometry)
        m_gridGeometry->updateGrid(m_gridBuffer.data.data(), m_gridW, m_gridH);

    buildAnomalyEntities(m_candidates);

    m_processing = false;
    m_progress = 1.0f;
    emit processingChanged();
    emit progressChanged();
}

void SurveyController::exportResults(const QString& format, const QString& path) {
    std::string p = path.toStdString();
    if (format == "CSV")
        AnomalyExporter::exportCsv(m_candidates, p);
    else
        AnomalyExporter::exportGeoJson(m_candidates, 51.5, -0.1, m_cellM, p);
}

void SurveyController::loadSession(int id) {
    auto buf = m_repository->loadSession(id, m_gridW, m_gridH, 1);
    m_gridBuffer = buf;
    if (m_gridGeometry)
        m_gridGeometry->updateGrid(m_gridBuffer.data.data(), m_gridW, m_gridH);
}

void SurveyController::clearAnomalyEntities() {
    for (auto* e : m_anomalyEntities) delete e;
    m_anomalyEntities.clear();
}

void SurveyController::buildAnomalyEntities(const std::vector<AnomalyCandidate>& candidates) {
    if (!m_sceneRoot) return;
    for (const auto& c : candidates) {
        auto* entity = new Qt3DCore::QEntity(m_sceneRoot);
        auto* mesh = new Qt3DExtras::QSphereMesh(entity);
        mesh->setRadius(0.3f);
        auto* mat = new Qt3DExtras::QPhongMaterial(entity);
        mat->setDiffuse(QColor(Qt::red));
        auto* tr = new Qt3DCore::QTransform(entity);
        tr->setTranslation({static_cast<float>(c.grid_x) * m_cellM,
                            c.peak_nT / 100.0f,
                            static_cast<float>(c.grid_y) * m_cellM});
        entity->addComponent(mesh);
        entity->addComponent(mat);
        entity->addComponent(tr);
        entity->setEnabled(m_showAnomalies);
        m_anomalyEntities.append(entity);
    }
}
