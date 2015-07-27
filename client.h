#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QUrl>
#include <QRectF>
#include <QQmlComponent>
#include <QQuickItem>

class Camera : public QObject
{
    Q_OBJECT
    Q_PROPERTY(float x READ x NOTIFY xChanged)
    Q_PROPERTY(float y READ y NOTIFY yChanged)
    Q_PROPERTY(float zoom READ zoom NOTIFY zoomChanged)

public:
    explicit Camera(QObject *parent = 0);

    float x() const;
    void setX(float x);

    float y() const;
    void setY(float y);

    float zoom() const;
    void setZoom(float zoom);

signals:
    void xChanged(float x);
    void yChanged(float y);
    void zoomChanged(float zoom);

protected:
    float m_x;
    float m_y;
    float m_zoom;
};

class Client : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QRectF border READ border NOTIFY borderChanged)
    Q_PROPERTY(QQmlComponent *nodeDelegate READ nodeDelegate WRITE setNodeDelegate NOTIFY nodeDelegateChanged)
    Q_PROPERTY(Camera *camera READ camera CONSTANT)

public:
    explicit Client(QObject *parent = 0);
    ~Client();

    Q_INVOKABLE void connectToHost(const QUrl &host);

    Q_INVOKABLE QQuickItem *node(quint32 nodeId) const;

    QRectF border() const;

    Camera *camera() const;

    void setNodeDelegate(QQmlComponent *nodeDelegate);
    QQmlComponent *nodeDelegate() const;

signals:
    void borderChanged(const QRectF &border);
    void nodeDelegateChanged(QQmlComponent *nodeDelegate);

public slots:

protected:
    class Private;
    friend class Private;
    Private *d;
};

#endif // CLIENT_H
