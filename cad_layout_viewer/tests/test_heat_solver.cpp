#include "heat_solver.h"
#include "heatmap_renderer.h"
#include "hotspot_analyser.h"
#include <QtTest/QtTest>
#include <QImage>
#include <cmath>
#include <vector>

class TestHeatSolver : public QObject {
    Q_OBJECT
private slots:

    void testSteadyStateFlatPlatform() {
        // Uniform κ, no source, Dirichlet BC left=100 right=0
        // After enough steps → linear T profile
        const int nx = 20, ny = 10;
        HeatSolver solver;
        HeatSolver::Params p;
        p.nx = nx; p.ny = ny;
        p.dt = 1e-4; p.n_steps = 5000;
        p.bc_left = 100.0; p.bc_right = 0.0;

        std::vector<double> kappa(static_cast<size_t>(nx * ny), 1.0);
        std::vector<double> q(static_cast<size_t>(nx * ny), 0.0);
        solver.solve(kappa, q, p);

        const auto& T = solver.temperature();
        // Check interior column at j=ny/2 is approximately linear
        for (int i = 1; i < nx - 1; ++i) {
            double expected = 100.0 - 100.0 * i / (nx - 1);
            double actual   = T[static_cast<size_t>((ny/2) * nx + i)];
            QVERIFY2(std::abs(actual - expected) < 5.0,
                qPrintable(QString("T[%1]=%2 expected~%3").arg(i).arg(actual).arg(expected)));
        }
    }

    void testHotspotDetectedAtCentre() {
        const int nx = 10, ny = 10;
        std::vector<double> T(static_cast<size_t>(nx * ny), 20.0);
        // Inject hotspot at (5,5)
        T[static_cast<size_t>(5 * nx + 5)] = 200.0;

        auto hs = HotspotAnalyser::detect(T, nx, ny, 200.0, 0.9);
        QVERIFY(!hs.isEmpty());
        QVERIFY(hs[0].max_temp >= 200.0 - 1e-6);
    }

    void testColorMapExtremes() {
        std::vector<double> T = {0.0, 100.0};
        QImage img = HeatmapRenderer::toQImage(T, 2, 1, 0.0, 100.0);
        QVERIFY(!img.isNull());
        // Cold pixel should be blueish (high blue component)
        QColor cold(img.pixel(0, 0));
        QVERIFY(cold.blue() > cold.red());
        // Hot pixel should be reddish (high red component)
        QColor hot(img.pixel(1, 0));
        QVERIFY(hot.red() > hot.blue());
    }

    void testSolverDoesNotProduceNaN() {
        const int nx = 8, ny = 8;
        HeatSolver solver;
        HeatSolver::Params p;
        p.nx = nx; p.ny = ny; p.dt = 1e-5; p.n_steps = 100;
        std::vector<double> kappa(static_cast<size_t>(nx * ny), 385.0);
        std::vector<double> q(static_cast<size_t>(nx * ny), 0.0);
        q[static_cast<size_t>((ny/2)*nx + nx/2)] = 1e6;
        solver.solve(kappa, q, p);
        for (double v : solver.temperature())
            QVERIFY2(std::isfinite(v), "NaN/Inf in temperature field");
    }
};

QTEST_GUILESS_MAIN(TestHeatSolver)
#include "test_heat_solver.moc"
