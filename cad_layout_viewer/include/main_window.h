#pragma once
#include "layout_engine.h"
#include "heat_solver.h"
#include "layout_canvas.h"
#include <QMainWindow>
#include <QThread>
#include <memory>

class QTreeWidget;
class QTableWidget;
class QProgressBar;
class QLabel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void openFile();
    void runSimulation();
    void clearSimulation();
    void showTutorial();
    void onLayerToggled();
    void onSimulationProgress(int pct);
    void onSimulationDone();

private:
    void buildMenuBar();
    void buildDockWidgets();
    void buildStatusBar();
    void populateLayerPanel();

    LayoutEngine   engine_;
    HeatSolver     solver_;
    LayoutCanvas*  canvas_;

    QTreeWidget*   layer_panel_{nullptr};
    QTableWidget*  hotspot_table_{nullptr};
    QProgressBar*  progress_{nullptr};
    QLabel*        fps_label_{nullptr};

    QThread*       sim_thread_{nullptr};
    HeatSolver::Params sim_params_;
};
