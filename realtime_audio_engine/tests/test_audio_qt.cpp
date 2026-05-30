#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QSignalSpy>
#include "PresetRepository.h"
#include "AudioEngineWrapper.h"

// ─── PresetRepository tests ───────────────────────────────────────────────────

class TestPresetRepository : public QObject {
    Q_OBJECT

    PresetRepository* repo_{nullptr};

private slots:
    void initTestCase() {
        repo_ = new PresetRepository(QStringLiteral("repo_test"), QStringLiteral(":memory:"));
        QVERIFY(repo_->init());
    }

    void testSaveAndLoad_RoundTrip() {
        QVERIFY(repo_->save({.id = -1, .name = QStringLiteral("warm"), .cutoff_hz = 1200.0f}));
        const auto list = repo_->loadAll();
        QCOMPARE(list.size(), 1);
        QCOMPARE(list[0].name, QStringLiteral("warm"));
        QVERIFY(std::abs(list[0].cutoff_hz - 1200.0f) < 0.01f);
    }

    void testUniqueName_Rejected() {
        // Second INSERT with the same name must fail (UNIQUE constraint).
        QVERIFY(repo_->save({.id = -1, .name = QStringLiteral("dup"), .cutoff_hz = 500.0f}));
        QVERIFY(!repo_->save({.id = -1, .name = QStringLiteral("dup"), .cutoff_hz = 600.0f}));
        QVERIFY(!repo_->lastError().isEmpty());
    }

    void testCutoffBelowRange_Rejected() {
        // CHECK constraint: cutoff_hz BETWEEN 20 AND 20000
        QVERIFY(!repo_->save({.id = -1, .name = QStringLiteral("bad"), .cutoff_hz = 5.0f}));
    }

    void testRemove_DeletesRow() {
        QVERIFY(repo_->save({.id = -1, .name = QStringLiteral("temp"), .cutoff_hz = 300.0f}));
        auto list = repo_->loadAll();
        int id = -1;
        for (const auto& p : list) {
            if (p.name == QStringLiteral("temp")) { id = p.id; break; }
        }
        QVERIFY(id >= 0);
        QVERIFY(repo_->remove(id));
        list = repo_->loadAll();
        const bool found = std::any_of(list.begin(), list.end(),
            [](const Preset& p){ return p.name == QStringLiteral("temp"); });
        QVERIFY(!found);
    }

    void cleanupTestCase() {
        delete repo_;
        repo_ = nullptr;
    }
};

// ─── AudioEngineWrapper tests ─────────────────────────────────────────────────

class TestAudioWrapper : public QObject {
    Q_OBJECT

    AudioEngineWrapper* wrapper_{nullptr};

private slots:
    void initTestCase() {
        // Use in-memory SQLite so tests leave no filesystem artefacts.
        wrapper_ = new AudioEngineWrapper(QStringLiteral(":memory:"));
    }

    void testCutoff_EmitsSignal() {
        QSignalSpy spy(wrapper_, &AudioEngineWrapper::cutoffHzChanged);
        wrapper_->setCutoffHz(500.0f);
        QCOMPARE(spy.count(), 1);
        QVERIFY(std::abs(wrapper_->cutoffHz() - 500.0f) < 0.01f);
    }

    void testCutoff_ClampsToRange() {
        wrapper_->setCutoffHz(0.0f);
        QVERIFY(wrapper_->cutoffHz() >= audio::kMinCutoffHz);
        wrapper_->setCutoffHz(999999.0f);
        QVERIFY(wrapper_->cutoffHz() <= audio::kMaxCutoffHz);
    }

    void testWaveform_PopulatesAfter300ms() {
        QTest::qWait(300);
        QCOMPARE(wrapper_->waveformBuffer().size(), static_cast<int>(audio::kWaveformSize));
    }

    void testPreset_RoundTrip() {
        wrapper_->setCutoffHz(800.0f);
        wrapper_->savePreset(QStringLiteral("test_preset"));
        const QList<Preset> all = wrapper_->presetModel()
                                       ->property("count").isValid()
                                 ? QList<Preset>{}
                                 : QList<Preset>{};
        // Access via repo directly is cleaner for this assertion.
        // Verify the wrapper's cutoff changes when a preset is loaded.
        wrapper_->setCutoffHz(1500.0f);  // change away first
        // The wrapper's loadPreset takes an id; fetch it from the model.
        const int rows = wrapper_->presetModel()->rowCount();
        QVERIFY(rows > 0);
        // Find the preset named "test_preset"
        for (int i = 0; i < rows; ++i) {
            const QModelIndex idx = wrapper_->presetModel()->index(i);
            if (wrapper_->presetModel()->data(idx, PresetModel::NameRole).toString()
                    == QStringLiteral("test_preset")) {
                const int id = wrapper_->presetModel()
                                   ->data(idx, PresetModel::IdRole).toInt();
                wrapper_->loadPreset(id);
                break;
            }
        }
        QVERIFY(std::abs(wrapper_->cutoffHz() - 800.0f) < 0.01f);
    }

    void testOverrun_DetectedAfterInject() {
        // Overrun injection is covered by AudioEngine.InjectOverrun_CountedInDiagnostics
        // in test_audio_core.cpp (GoogleTest).  The Qt poll path emitting overrunsChanged
        // would require exposing testInjectOverrun via Q_INVOKABLE — acceptable future work.
        QSKIP("Overrun injection tested in test_audio_core (GoogleTest)");
    }

    void cleanupTestCase() {
        delete wrapper_;
        wrapper_ = nullptr;
    }
};

// ─── main ─────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("audio_qt_tests"));
    app.setOrganizationName(QStringLiteral("portfolio"));

    int status = 0;
    {
        TestPresetRepository t;
        status |= QTest::qExec(&t, argc, argv);
    }
    {
        TestAudioWrapper t;
        status |= QTest::qExec(&t, argc, argv);
    }
    return status;
}

#include "test_audio_qt.moc"
