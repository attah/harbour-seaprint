#include "overrider.h"
#include <QMutex>
#include <QStandardPaths>
#include <QtDebug>

Overrider::Overrider()
{
//    QFile file(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)+"/overrides");
    QFile file(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+"/.seaprint_overrides");
    qDebug() << "AAAAAAAAAAAAAAA" << file.fileName();
    if(file.open(QIODevice::ReadOnly))
    {
        QJsonDocument JsonDocument = QJsonDocument::fromJson(file.readAll());

        _overrides = JsonDocument.object();
        qDebug() << "overides loaded" << _overrides;
        file.close();
    }
}

Overrider::~Overrider() {
}

Overrider* Overrider::m_Instance = nullptr;

Overrider* Overrider::instance()
{
    static QMutex mutex;
    if (!m_Instance)
    {
        mutex.lock();

        if (!m_Instance)
            m_Instance = new Overrider();

        mutex.unlock();
    }

    return m_Instance;
}

bool Overrider::apply(QJsonObject& attrs)
{
    bool applied = false;
    foreach(const QString& key, _overrides.keys())
    {
        if(attrs.contains(key))
        {
            QString match = attrs[key].toObject()["value"].toString();
            if(match != "" && _overrides[key].toObject().contains(match))
            {
                QJsonObject overrideden_data = _overrides[key].toObject()[match].toObject();
                foreach(const QString& o_key, overrideden_data.keys())
                {
                    attrs.insert(o_key, overrideden_data[o_key]);
                }
                applied=true;
            }
        }
    }
    return applied;
}
