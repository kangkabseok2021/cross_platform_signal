#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "CtImageProvider.h"
#include "ReconstructionController.h"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

    auto* provider    = new CtImageProvider();
    auto* controller  = new ReconstructionController(provider);

    QQmlApplicationEngine engine;
    engine.addImageProvider("ct", provider);  // image://ct/slice
    engine.rootContext()->setContextProperty("controller", controller);

    engine.loadFromModule("CtViewer", "Main");

    if (engine.rootObjects().isEmpty()) return -1;
    return QGuiApplication::exec();
}
