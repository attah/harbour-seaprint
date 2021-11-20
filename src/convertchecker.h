#ifndef CONVERTCHECKER_H
#define CONVERTCHECKER_H
#include <QObject>
#include <QMutex>
#include "madness.h"

class ConvertChecker : public QObject
{
    Q_OBJECT
public:
    static ConvertChecker* instance();
    Q_PROPERTY(bool pdf MEMBER _pdf CONSTANT)
    Q_PROPERTY(bool calligra MEMBER _calligra CONSTANT)

    Q_INVOKABLE int pdfPages(QString pdf);

signals:
protected:
private:
    static ConvertChecker* m_Instance;

    ConvertChecker();
    ~ConvertChecker();
    ConvertChecker(const ConvertChecker &);
    ConvertChecker& operator=(const ConvertChecker &);

    LibLoader poppler;
    bool _pdf;
    bool _calligra;
};

#endif // CONVERTCHECKER_H
