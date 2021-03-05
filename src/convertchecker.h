#ifndef CONVERTCHECKER_H
#define CONVERTCHECKER_H
#include <QObject>
#include <QMutex>

class ConvertChecker : public QObject
{
    Q_OBJECT
public:
    static ConvertChecker* instance();
    Q_PROPERTY(bool pdf MEMBER _pdf)
    Q_PROPERTY(bool calligra MEMBER _calligra)

    Q_INVOKABLE quint32 pdfPages(QString pdf);

signals:
protected:
private:
    static ConvertChecker* m_Instance;

    ConvertChecker();
    ~ConvertChecker();
    ConvertChecker(const ConvertChecker &);
    ConvertChecker& operator=(const ConvertChecker &);

    bool _pdf;
    bool _calligra;
};

#endif // CONVERTCHECKER_H
