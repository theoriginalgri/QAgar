TEMPLATE = app

QT += qml quick websockets network

SOURCES += main.cpp \
    client.cpp \
    node.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    client.h \
    node.h
