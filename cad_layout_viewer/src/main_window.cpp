#include "main_window.h"
#include <QApplication>
#include "heatmap_renderer.h"
#include "hotspot_analyser.h"
#include "tutorial_wizard.h"
#include <QAction>
#include <QCheckBox>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QSpinBox>
#include <QStatusBar>
#include <QTableWidget>
#include <QToolBar>
#include <QTreeWidget>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    canvas_ = new LayoutCanvas(this);
    setCentralWidget(canvas_);
    buildMenuBar();
    buildDockWidgets();
    buildStatusBar();
    resize(1280, 800);
    setWindowTitle(tr("CAD Layout Viewer & Simulator"));
}

void MainWindow::buildMenuBar() {
    auto* file = menuBar()->addMenu(tr("&Datei"));
    file->addAction(tr("&Öffnen..."), this, &MainWindow::openFile, QKeySequence::Open);
    file->addSeparator();
    file->addAction(tr("&Beenden"), qApp, &QApplication::quit);

    auto* view = menuBar()->addMenu(tr("&Ansicht"));
    view->addAction(tr("An Fenster anpassen"), canvas_, &LayoutCanvas::fitAll);

    auto* sim = menuBar()->addMenu(tr("&Simulation"));
    sim->addAction(tr("Simulation &starten"),  this, &MainWindow::runSimulation);
    sim->addAction(tr("Simulation &löschen"),  this, &MainWindow::clearSimulation);

    auto* help = menuBar()->addMenu(tr("&Hilfe"));
    help->addAction(tr("Tutorial..."), this, &MainWindow::showTutorial);
}

void MainWindow::buildDockWidgets() {
    // Layer panel
    auto* dock_l = new QDockWidget(tr("Layer"), this);
    layer_panel_ = new QTreeWidget;
    layer_panel_->setHeaderLabel(tr("Layer"));
    connect(layer_panel_, &QTreeWidget::itemChanged, this, &MainWindow::onLayerToggled);
    dock_l->setWidget(layer_panel_);
    addDockWidget(Qt::LeftDockWidgetArea, dock_l);

    // Properties panel + simulation params
    auto* dock_r  = new QDockWidget(tr("Eigenschaften"), this);
    auto* form_w  = new QWidget;
    auto* form    = new QFormLayout(form_w);

    auto* dt_spin = new QDoubleSpinBox;
    dt_spin->setRange(1e-6, 1.0); dt_spin->setValue(sim_params_.dt);
    dt_spin->setDecimals(6);
    connect(dt_spin, &QDoubleSpinBox::valueChanged,
            [this](double v){ sim_params_.dt = v; });
    form->addRow(tr("Δt (s)"), dt_spin);

    auto* steps_spin = new QSpinBox;
    steps_spin->setRange(1, 10000); steps_spin->setValue(sim_params_.n_steps);
    connect(steps_spin, &QSpinBox::valueChanged,
            [this](int v){ sim_params_.n_steps = v; });
    form->addRow(tr("Schritte"), steps_spin);

    progress_ = new QProgressBar;
    progress_->setRange(0, 100); progress_->setValue(0);
    form->addRow(tr("Fortschritt"), progress_);

    // Hotspot table
    hotspot_table_ = new QTableWidget(0, 3);
    hotspot_table_->setHorizontalHeaderLabels({tr("x"), tr("y"), tr("T_max (°C)")});
    auto* vl = new QVBoxLayout;
    vl->addLayout(form);
    vl->addWidget(new QLabel(tr("Hotspots")));
    vl->addWidget(hotspot_table_);
    form_w->setLayout(vl);
    dock_r->setWidget(form_w);
    addDockWidget(Qt::RightDockWidgetArea, dock_r);
}

void MainWindow::buildStatusBar() {
    fps_label_ = new QLabel(tr("FPS: --"));
    statusBar()->addPermanentWidget(fps_label_);
}

void MainWindow::openFile() {
    QString path = QFileDialog::getOpenFileName(
        this, tr("Layout öffnen"), {}, tr("JSON Layout (*.json);;Alle Dateien (*)"));
    if (path.isEmpty()) return;
    try {
        engine_.load(path);
        canvas_->setLayout(&engine_);
        canvas_->clearHeatmap();
        populateLayerPanel();
        hotspot_table_->setRowCount(0);
        statusBar()->showMessage(tr("Geladen: %1").arg(path));
    } catch (const std::exception& ex) {
        QMessageBox::critical(this, tr("Fehler"), QString::fromStdString(ex.what()));
    }
}

void MainWindow::populateLayerPanel() {
    layer_panel_->clear();
    for (auto id : engine_.layerIds()) {
        auto* item = new QTreeWidgetItem(layer_panel_);
        item->setText(0, tr("Layer %1").arg(id));
        item->setData(0, Qt::UserRole, id);
        item->setCheckState(0, Qt::Checked);
    }
}

void MainWindow::onLayerToggled() {
    for (int i = 0; i < layer_panel_->topLevelItemCount(); ++i) {
        auto* item = layer_panel_->topLevelItem(i);
        uint32_t id = item->data(0, Qt::UserRole).toUInt();
        canvas_->setLayerVisible(id, item->checkState(0) == Qt::Checked);
    }
}

void MainWindow::runSimulation() {
    if (engine_.isEmpty()) return;
    QRectF bb = engine_.bbox();
    if (bb.isEmpty()) return;

    const int nx = sim_params_.nx, ny = sim_params_.ny;
    // Build uniform κ field (copper 385 W/mK as default)
    std::vector<double> kappa(static_cast<size_t>(nx * ny), 385.0);
    std::vector<double> q(static_cast<size_t>(nx * ny), 0.0);
    // Inject heat source at centre
    q[static_cast<size_t>((ny/2)*nx + nx/2)] = 1e9;

    progress_->setValue(0);
    solver_.solve(kappa, q, sim_params_);
    progress_->setValue(100);

    QImage hm = HeatmapRenderer::toQImage(
        solver_.temperature(), nx, ny, solver_.tMin(), solver_.tMax());
    canvas_->setHeatmap(hm);

    auto hotspots = HotspotAnalyser::detect(
        solver_.temperature(), nx, ny, solver_.tMax());
    hotspot_table_->setRowCount(hotspots.size());
    for (int r = 0; r < hotspots.size(); ++r) {
        hotspot_table_->setItem(r,0, new QTableWidgetItem(
            QString::number(hotspots[r].centroid.x(), 'f', 1)));
        hotspot_table_->setItem(r,1, new QTableWidgetItem(
            QString::number(hotspots[r].centroid.y(), 'f', 1)));
        hotspot_table_->setItem(r,2, new QTableWidgetItem(
            QString::number(hotspots[r].max_temp,      'f', 2)));
    }
}

void MainWindow::clearSimulation() {
    canvas_->clearHeatmap();
    hotspot_table_->setRowCount(0);
    progress_->setValue(0);
}

void MainWindow::showTutorial() {
    TutorialWizard wiz(this);
    wiz.exec();
}

void MainWindow::onSimulationProgress(int pct) { progress_->setValue(pct); }
void MainWindow::onSimulationDone()            { progress_->setValue(100); }
