#include "client.h"

#include <QWebSocket>
#include <QAbstractSocket>
#include <QDataStream>
#include <QDebug>
#include <QPointer>
#include <QQuickItem>
#include <QQmlContext>
#include <QHash>

enum PacketType
{
    UpdateNodes = 16,
    UpdatePositionAndSize = 17,
    UpdateLoaderboard = 49,
    SetBorder = 64
};

enum NodeDataFlag
{
    Virus = 1,
    Advance4 = 2,
    Advance8 = 4,
    Advance16 = 8,
    AgitatedVirus = 16
};

typedef QFlags<NodeDataFlag> NodeDataFlags;

struct NodeData {
    quint32 id;

    quint32 x, y;
    quint16 radius;

    quint8 r, g, b;

    quint8 flags;

    QString name;
};

class Client::Private : public QObject
{
    Q_OBJECT

public:
    Private(Client *q)
    : q(q)
    , ws("https://localhost")
    {
        ws.ignoreSslErrors();

        connect(&ws, SIGNAL(connected()), this, SLOT(connected()));
        connect(&ws, SIGNAL(disconnected()), this, SLOT(disconnected()));
        connect(&ws, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError()));
        connect(&ws, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(binaryMessageReceived(QByteArray)));
    }

    Client *q;

    QWebSocket ws;

    QRectF border;
    QPointer<QQmlComponent> nodeDelegate;

    QPointer<QQuickItem> camera;

    QHash<quint32, QQuickItem *> nodeMap;

public:
    void parsePacket(QDataStream &stream);

    void sendSpectate();
    void sendInit();

    void updateCamera(float x, float y);

    QQuickItem *getOrCreateNode(quint32 id);

public slots:
    void connected();
    void disconnected();
    void onError();

    void binaryMessageReceived(const QByteArray &message);
};

void Client::Private::parsePacket(QDataStream &stream)
{
    quint8 packetId;
    stream >> packetId;

    switch(packetId) {
    case UpdateNodes:
        {
            quint16 numberOfNodesDestroyed;
            stream >> numberOfNodesDestroyed;

            for(quint16 i=0; i < numberOfNodesDestroyed; ++i) {
                quint32 killerId, nodeId;
                stream >> killerId >> nodeId;

                //qDebug() << "Node killed:" << nodeId << "by" << killerId;

                QQuickItem *item = nodeMap.value(nodeId);
                if (item) {
                    item->deleteLater();
                    nodeMap.remove(nodeId);
                }
            }

            QList<NodeData> nodeDataList;
            while(true) {
                NodeData nd;

                stream >> nd.id;

                if (nd.id == 0)
                    break;

                stream >> nd.x >> nd.y
                       >> nd.radius
                       >> nd.r >> nd.g >> nd.b
                       >> nd.flags;

                NodeDataFlags flags(nd.flags);
                if (flags.testFlag(Advance4))
                    stream.skipRawData(4);
                else if (flags.testFlag(Advance8))
                    stream.skipRawData(8);
                else if (flags.testFlag(Advance16))
                    stream.skipRawData(16);

                quint16 c;

                do {
                    stream >> c;

                    if (c)
                        nd.name += QChar(c);
                } while (c);

                //stream >> nd.name;

                //quint16 endOfString;
                //stream >> endOfString;

                QQuickItem *item = getOrCreateNode(nd.id);
                item->setPosition(QPointF(nd.x, nd.y));
                item->setWidth(nd.radius*2);
                item->setHeight(nd.radius*2);

                item->setProperty("color", QColor(nd.r, nd.g, nd.b));
                item->setProperty("name", nd.name);

                nodeDataList.append(nd);
            }

            quint16 noIdea;
            stream >> noIdea;

            if (noIdea != 0) {
                break;
            }

            quint32 numberOfNodesMarkedDestroying;
            stream >> numberOfNodesMarkedDestroying;

            for(quint32 i=0; i < numberOfNodesMarkedDestroying; ++i) {
                quint32 nodeId;
                stream >> nodeId;

                QQuickItem *item = nodeMap.value(nodeId);
                if (item) {
                    item->deleteLater();
                    nodeMap.remove(nodeId);
                }

                //qDebug() << "Node destroyed:" << nodeId;
            }
        }
        break;

    case UpdatePositionAndSize:
        {
            float x, y, zoom;
            stream >> x >> y >> zoom;

            updateCamera(x, y);

            //qDebug() << QString("Position changed: x=%1 y=%2 zoom=%3").arg(x).arg(y).arg(zoom);
        }
        break;

    case UpdateLoaderboard:
        {
            quint32 number;
            stream >> number;

            for(quint32 i=0; i < number; ++i) {
                quint32 id;
                QString name;

                stream >> id;

                quint16 c;
                do {
                    stream >> c;

                    if (c)
                        name += QChar(c);
                } while (c);

                qDebug() << "Leaderboard:" << i << id << name;
            }
        }
        break;

    case SetBorder:
        {
            stream.setFloatingPointPrecision(QDataStream::DoublePrecision);

            double left, top, right, bottom;
            stream >> left >> top >> right >> bottom;

            border.setCoords(left, top, right, bottom);
            emit q->borderChanged(border);

            qDebug() << "Bounds:" << border;
        }
        break;

    default:
        qDebug() << "Unknown packet type:" << packetId;
    }
}

void Client::Private::sendSpectate()
{
    QByteArray data;
    QDataStream ds(&data, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::LittleEndian);

    quint8 packetId = 1;
    ds << packetId;

    ws.sendBinaryMessage(data);
}

void Client::Private::sendInit()
{
    QByteArray data;
    QDataStream ds(&data, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::LittleEndian);

    /*
    ds << (quint8)254 << (quint32)5;
    ws.sendBinaryMessage(data);

    data.clear();
    */

    ds << (quint8)255 << (quint32)1;
    ws.sendBinaryMessage(data);
}

void Client::Private::updateCamera(float x, float y)
{
    if (!camera) {
        QQmlContext *context = qmlContext(q);

        //camera = qobject_cast<QQuickItem *>(nodeDelegate->create(context));
        //camera->setParentItem(boardItem);
    }

    //camera->setPosition(QPointF(x, y));
}

QQuickItem *Client::Private::getOrCreateNode(quint32 id)
{
    QQuickItem *item = nodeMap.value(id);

    if (!item) {
        QQmlContext *context = qmlContext(q);

        //qDebug() << "Node" << id << "created";

        item = qobject_cast<QQuickItem *>(nodeDelegate->create(context));

        nodeMap.insert(id, item);
    }

    return item;
}

void Client::Private::connected()
{
    qDebug() << Q_FUNC_INFO;

    sendInit();
    sendSpectate();
}

void Client::Private::disconnected()
{
    qDebug() << Q_FUNC_INFO;
}

void Client::Private::onError()
{
    qDebug() << Q_FUNC_INFO << ws.errorString();
}

void Client::Private::binaryMessageReceived(const QByteArray &message)
{
    QDataStream stream(message);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    parsePacket(stream);
}

Client::Client(QObject *parent)
: QObject(parent)
, d(new Private(this))
{
}

Client::~Client()
{
    delete d;
}

void Client::connectToHost(const QUrl &host)
{
    d->ws.open(host);
}

QRectF Client::border() const
{
    return d->border;
}

void Client::setNodeDelegate(QQmlComponent *nodeDelegate)
{
    if (d->nodeDelegate == nodeDelegate)
        return;

    d->nodeDelegate = nodeDelegate;
    emit nodeDelegateChanged(nodeDelegate);
}

QQmlComponent *Client::nodeDelegate() const
{
    return d->nodeDelegate;
}

#include "client.moc"


