#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "TelemetryModel.h"
#include "TelemetryHttpServer.h"
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <string>

int main(int argc, char** argv) {
    QGuiApplication app(argc, argv);

    uint16_t port = 8080;
    for (int i = 1; i < argc - 1; ++i) {
        if (std::string(argv[i]) == "--port")
            port = static_cast<uint16_t>(std::atoi(argv[i + 1]));
    }

    auto* model = new TelemetryModel(&app);

    TelemetryHttpServer server(
        [model]() { return model->telemetrySnapshot(); },
        [model](int ch, double a) { model->setAlpha(ch, a); }
    );
    server.start(port);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("Telemetry"), model);
    const QUrl url(QStringLiteral("qrc:/qt/qml/VehicleTelemetry/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject* obj, const QUrl& u) {
            if (!obj && u == url) QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);

    int result = app.exec();
    server.stop();
    return result;
}
