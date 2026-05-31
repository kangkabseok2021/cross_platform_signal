#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "AcousticViewModel.h"

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    AcousticViewModel vm;
    engine.rootContext()->setContextProperty("acousticModel", &vm);
    engine.loadFromModule("AcousticEngine", "Main");

    return app.exec();
}
