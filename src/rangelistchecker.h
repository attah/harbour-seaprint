#ifndef RANGELISTCHECKER_H
#define RANGELISTCHECKER_H

#include <QValidator>
#include <QMutex>
#include <QJsonArray>
#include <QJsonObject>

#include "printparameters.h"

class RangeListChecker : public QObject
{
    Q_OBJECT
public:
    static RangeListChecker* instance();

    Q_INVOKABLE QJsonArray parse(QString str) const;

private:
    static RangeListChecker* m_Instance;

    RangeListChecker();
    ~RangeListChecker();
    RangeListChecker(const RangeListChecker &);
    RangeListChecker& operator=(const RangeListChecker &);

};

#endif // RANGELISTCHECKER_H
