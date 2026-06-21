#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DCore/QEntity>
#include <Qt3DExtras/QOrbitCameraController>
#include <Qt3DRender/QCamera>
#include <Qt3DExtras/QPerVertexColorMaterial>
#include <Qt3DCore/QGeometryView>
#include <Qt3DRender/QGeometryRenderer>
#include "SurveyController.h"
#include "SurveyGridGeometry.h"

using namespace Qt::StringLiterals;

int main(int argc, char** argv) {
    QGuiApplication app(argc, argv);
    app.setApplicationName("SensysVisualizer");
    app.setOrganizationName("Portfolio");

    auto* view = new Qt3DExtras::Qt3DWindow();
    view->defaultFrameGraph()->setClearColor(QColor(30, 30, 40));

    auto* root = new Qt3DCore::QEntity();

    auto* geo = new SurveyGridGeometry(root);
    auto* geoView = new Qt3DCore::QGeometryView(root);
    geoView->setGeometry(geo);
    geoView->setPrimitiveType(Qt3DCore::QGeometryView::Triangles);

    auto* renderer = new Qt3DRender::QGeometryRenderer(root);
    renderer->setView(geoView);

    auto* gridEntity = new Qt3DCore::QEntity(root);
    auto* mat = new Qt3DExtras::QPerVertexColorMaterial(gridEntity);
    gridEntity->addComponent(renderer);
    gridEntity->addComponent(mat);

    auto* cam = view->camera();
    cam->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    cam->setPosition(QVector3D(10, 30, 30));
    cam->setViewCenter(QVector3D(10, 0, 10));

    auto* camCtrl = new Qt3DExtras::QOrbitCameraController(root);
    camCtrl->setLinearSpeed(50.0f);
    camCtrl->setLookSpeed(180.0f);
    camCtrl->setCamera(cam);

    view->setRootEntity(root);

    auto* controller = new SurveyController();
    controller->setSceneRoot(root);
    controller->setGridGeometry(geo);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("surveyController", controller);
    engine.rootContext()->setContextProperty("view3d",
        QVariant::fromValue(static_cast<QObject*>(view)));

    const QUrl url(u"qrc:/SensysApp/qml/main.qml"_s);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject* obj, const QUrl& objUrl) {
            if (!obj && url == objUrl) QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
