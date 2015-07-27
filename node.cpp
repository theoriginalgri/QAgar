#include "node.h"

Node::Node(QObject *parent)
: QObject(parent)
{
}

quint32 Node::id() const
{
    return m_id;
}

void Node::setId(const quint32 &id)
{
    if (m_id == id)
        return;

    m_id = id;
    emit idChanged(id);
}

quint32 Node::x() const
{
    return m_x;
}

void Node::setX(const quint32 &x)
{
    if (m_x == x)
        return;

    m_x = x;
    emit xChanged(x);
}

quint32 Node::y() const
{
    return m_y;
}

void Node::setY(const quint32 &y)
{
    if (m_y == y)
        return;

    m_y = y;
    emit yChanged(y);
}

quint16 Node::mass() const
{
    return m_mass;
}

void Node::setMass(const quint16 &mass)
{
    if (m_mass == mass)
        return;

    m_mass = mass;
    emit massChanged(mass);
}

QColor Node::color() const
{
    return m_color;
}

void Node::setColor(const QColor &color)
{
    if (m_color == color)
        return;

    m_color = color;
    emit colorChanged(color);
}

quint8 Node::flags() const
{
    return m_flags;
}

void Node::setFlags(const quint8 &flags)
{
    m_flags = flags;
}

QString Node::name() const
{
    return m_name;
}

void Node::setName(const QString &name)
{
    if (m_name == name)
        return;

    m_name = name;
    emit nameChanged(name);
}


