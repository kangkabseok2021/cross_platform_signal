#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "AudioEngineWrapper.h"

using namespace Qt::StringLiterals;

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName(u"AudioEngine"_s);
    app.setOrganizationName(u"portfolio"_s);

    QQmlApplicationEngine engine;
    AudioEngineWrapper wrapper;
    engine.rootContext()->setContextProperty(u"Engine"_s, &wrapper);

    const QUrl url(u"qrc:/AudioEngine/qml/main.qml"_s);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject* obj, const QUrl& objUrl) {
        if (!obj && url == objUrl) QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
