#ifndef MIMER_H
#define MIMER_H

#include <QString>
#include <QObject>
#include <QMutex>
#include <QMimeDatabase>

class Mimer : public  QObject
{
    Q_OBJECT
public:
    static Mimer* instance();
    Q_INVOKABLE QString get_type(QString filename);
private:
    Mimer();
    static Mimer* m_Instance;
    QMimeDatabase _db;

};

#endif // MIMER_H
