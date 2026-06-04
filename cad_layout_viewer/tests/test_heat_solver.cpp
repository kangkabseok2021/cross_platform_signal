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
        // Uniform κ=1, no source, Dirichlet BC left=100 right=0.
        // After convergence: T must decrease monotonically left→right.
        // (Exact linear profile only holds in 1D; 2D top/bottom BCs at 0
        // reduce interior temperatures, so we check monotonicity instead.)
        const int nx = 5, ny = 3;
        HeatSolver solver;
        HeatSolver::Params p;
        p.nx = nx; p.ny = ny;
        p.dt = 0.1; p.n_steps = 3000;   // CFL stable: dt=0.1 < dx²/(4κ)=0.25
        p.bc_left = 100.0; p.bc_right = 0.0;

        std::vector<double> kappa(static_cast<size_t>(nx * ny), 1.0);
        std::vector<double> q(static_cast<size_t>(nx * ny), 0.0);
        solver.solve(kappa, q, p);

        const auto& T = solver.temperature();
        // Left boundary must be hotter than right boundary
        QVERIFY(T[static_cast<size_t>((ny/2)*nx + 0)] > T[static_cast<size_t>((ny/2)*nx + nx-1)]);
        // Temperature must decrease monotonically along the middle row
        for (int i = 0; i < nx - 1; ++i)
            QVERIFY2(T[static_cast<size_t>((ny/2)*nx + i)] >=
                     T[static_cast<size_t>((ny/2)*nx + i + 1)] - 1e-6,
                qPrintable(QString("T[%1]=%2 < T[%3]=%4 — not monotone")
                    .arg(i).arg(T[static_cast<size_t>((ny/2)*nx+i)])
                    .arg(i+1).arg(T[static_cast<size_t>((ny/2)*nx+i+1)])));
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
