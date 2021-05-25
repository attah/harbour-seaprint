#ifndef OVERRIDER_H
#define OVERRIDER_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>

class Overrider
{
public:
    static Overrider* instance();
    bool apply(QJsonObject& attrs);

private:
    Overrider();
    ~Overrider();
    Overrider(const Overrider &);
    Overrider& operator=(const Overrider &);

    static Overrider* m_Instance;
    QJsonObject _overrides;
};

#endif // OVERRIDER_H
