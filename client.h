#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QUrl>
#include <QRectF>
#include <QQmlComponent>

class Client : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QRectF border READ border NOTIFY borderChanged)
    Q_PROPERTY(QQmlComponent *nodeDelegate READ nodeDelegate WRITE setNodeDelegate NOTIFY nodeDelegateChanged)

public:
    explicit Client(QObject *parent = 0);
    ~Client();

    Q_INVOKABLE void connectToHost(const QUrl &host);

    QRectF border() const;

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
