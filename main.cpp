#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>

#include <QQmlComponent>
#include <QQuickWindow>

#include "imageHandler.hpp"

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);


    qmlRegisterType<ImageHandler>("an.qt.ImageHandler", 1, 0, "ImageHandler");
    //    QQmlApplicationEngine engine;
    //    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    //    if (engine.rootObjects().isEmpty())
    //        return -1;

    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.loadUrl(QUrl(QStringLiteral("qrc:/main.qml")));



    qDebug() << "Component error :"<< component.errors();
    QQuickWindow *window;
    window = qobject_cast<QQuickWindow *>(component.create());
    window->show();

    QObject::connect(&engine,SIGNAL(quit()),&app,SLOT(quit()));

    //string imageDir="C:/develop/test/pictures/lena.jpg";

    // ImageHandler *imageHandler= new ImageHandler(imageDir);
    //imageHandler->handle();
    //return 0;
    return app.exec();
}
