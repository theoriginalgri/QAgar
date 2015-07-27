#ifndef NODE_H
#define NODE_H

#include <QObject>
#include <QColor>
#include <QtQml>

class Node : public QObject
{
    Q_OBJECT
    Q_PROPERTY(quint32 id READ id NOTIFY idChanged)
    Q_PROPERTY(quint32 x READ x NOTIFY xChanged)
    Q_PROPERTY(quint32 y READ y NOTIFY yChanged)
    Q_PROPERTY(quint16 mass READ mass NOTIFY massChanged)
    Q_PROPERTY(QColor color READ color NOTIFY colorChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)

public:
    explicit Node(QObject *parent = 0);

    quint32 id() const;
    void setId(const quint32 &id);

    quint32 x() const;
    void setX(const quint32 &x);

    quint32 y() const;
    void setY(const quint32 &y);

    quint16 mass() const;
    void setMass(const quint16 &mass);

    QColor color() const;
    void setColor(const QColor &color);

    quint8 flags() const;
    void setFlags(const quint8 &flags);

    QString name() const;
    void setName(const QString &name);

signals:
    void idChanged(const quint32 &id);
    void xChanged(const quint32 &x);
    void yChanged(const quint32 &y);
    void massChanged(const quint16 &mass);
    void colorChanged(const QColor &color);
    void nameChanged(const QString &name);

protected:
    quint32 m_id;

    quint32 m_x, m_y;
    quint16 m_mass;

    QColor m_color;

    quint8 m_flags;

    QString m_name;
};

QML_DECLARE_TYPE(Node)

#endif // NODE_H
