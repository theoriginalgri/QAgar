#include "client.h"
#include "node.h"

#include <QWebSocket>
#include <QAbstractSocket>
#include <QDataStream>
#include <QDebug>
#include <QPointer>
#include <QQuickItem>
#include <QQmlContext>
#include <QHash>

Camera::Camera(QObject *parent)
: QObject(parent)
, m_x(0)
, m_y(0)
, m_zoom(0)
{
}

float Camera::x() const
{
    return m_x;
}

void Camera::setX(float x)
{
    if (m_x == x)
        return;

    m_x = x;
    emit xChanged(x);
}

float Camera::y() const
{
    return m_y;
}

void Camera::setY(float y)
{
    if (m_y == y)
        return;

    m_y = y;
    emit yChanged(y);
}

float Camera::zoom() const
{
    return m_zoom;
}

void Camera::setZoom(float zoom)
{
    if (m_zoom == zoom)
        return;

    m_zoom = zoom;
    emit zoomChanged(zoom);
}

enum PacketType
{
    UpdateNodes = 16,
    UpdatePositionAndSize = 17,
    UpdateLeaderboard = 49,
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
    , camera(this)
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

    Camera camera;

    QHash<quint32, QQuickItem *> nodeMap;

public:
    void parsePacket(QDataStream &stream);

    void sendSpectate();
    void sendInit();

    void updateCamera(float x, float y, float zoom);

    QQuickItem *getOrCreateNode(const NodeData &nd);

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

                QQuickItem *item = getOrCreateNode(nd);
                item->setPosition(QPointF(nd.x, nd.y));
                item->setWidth(nd.radius*2);
                item->setHeight(nd.radius*2);
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

            updateCamera(x, y, zoom);
        }
        break;

    case UpdateLeaderboard:
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

                //qDebug() << "Leaderboard:" << i << id << name;
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

void Client::Private::updateCamera(float x, float y, float zoom)
{
    camera.setX(x);
    camera.setY(y);
    camera.setZoom(zoom);
}

QQuickItem *Client::Private::getOrCreateNode(const NodeData &nd)
{
    QQuickItem *item = nodeMap.value(nd.id);

    if (!item) {
        QQmlContext *context = new QQmlContext(qmlContext(q));

        Node *node = new Node;
        node->setX(nd.x);
        node->setY(nd.y);
        node->setMass(nd.radius);
        node->setColor(QColor(nd.r, nd.g, nd.b));
        node->setFlags(nd.flags);
        node->setName(nd.name);

        context->setContextProperty("node", node);
        context->setContextObject(node);

        item = qobject_cast<QQuickItem *>(nodeDelegate->create(context));

        nodeMap.insert(nd.id, item);
    } else {
        Node *node = qvariant_cast<Node *>(qmlContext(item)->contextProperty("node"));

        node->setX(nd.x);
        node->setY(nd.y);
        node->setMass(nd.radius);
        node->setColor(QColor(nd.r, nd.g, nd.b));
        node->setFlags(nd.flags);
        //node->setName(name);
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

QQuickItem *Client::node(quint32 nodeId) const
{
    return d->nodeMap.value(nodeId);
}

QRectF Client::border() const
{
    return d->border;
}

Camera *Client::camera() const
{
    return &d->camera;
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
