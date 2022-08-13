#include "rangelistchecker.h"

RangeListChecker::RangeListChecker()
{

}

RangeListChecker::~RangeListChecker()
{

}

RangeListChecker* RangeListChecker::m_Instance = 0;

RangeListChecker* RangeListChecker::instance()
{
    static QMutex mutex;
    if (!m_Instance)
    {
        mutex.lock();

        if (!m_Instance)
            m_Instance = new RangeListChecker;

        mutex.unlock();
    }

    return m_Instance;
}

QJsonArray RangeListChecker::parse(QString str) const
{
    PrintParameters params;
    params.setPageRange(str.toStdString());
    QJsonArray ret;
    for(const std::pair<size_t, size_t>& p : params.pageRangeList)
    {
        ret.append(QJsonObject {{"low", int(p.first)}, {"high", int(p.second)}});
    }
    return ret;
}
