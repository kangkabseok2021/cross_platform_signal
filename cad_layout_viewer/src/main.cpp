#include "main_window.h"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("CAD Layout Viewer");
    app.setApplicationVersion("1.0.0");
    MainWindow w;
    w.show();
    return app.exec();
}
