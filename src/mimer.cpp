#include "mimer.h"

Mimer::Mimer()
{

}

Mimer* Mimer::m_Instance = 0;

Mimer* Mimer::instance()
{
    static QMutex mutex;
    if (!m_Instance)
    {
        mutex.lock();

        if (!m_Instance)
            m_Instance = new Mimer;

        mutex.unlock();
    }

    return m_Instance;
}

QString Mimer::get_type(QString filename) {
    return _db.mimeTypeForFile(filename).name();
}
