#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlEngine>
#include <QtQml>

#include "client.h"
#include "node.h"

QML_DECLARE_TYPE(Client)
QML_DECLARE_TYPE(Camera)

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<Client>("agar", 1, 0, "Client");
    qmlRegisterType<Node>("agar", 1, 0, "Node");
    qmlRegisterType<Camera>("agar", 1, 0, "Camera");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
