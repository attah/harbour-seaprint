#ifndef QIPPDISCOVERY_H
#define QIPPDISCOVERY_H
#include <QStringListModel>
#include <QThread>
#include <QDebug>

#include "bytestream.h"
#include "ippdiscovery.h"
#include "lthread.h"

#include <utility>

class QIppDiscoveryWorker : public QObject
{
    Q_OBJECT
public:
    QIppDiscoveryWorker()
    : _ippDiscovery(new IppDiscovery([this](const std::string& addr){wasFound(QString::fromStdString(addr));}))
    {}

public slots:
    void discover(bool clean)
    {
        if(clean)
        {
            _ippDiscovery.reset(new IppDiscovery([this](const std::string& addr){wasFound(QString::fromStdString(addr));}));
        }
        qDebug() << "actually discovering";
        _ippDiscovery->discover();
        qDebug() << "done actually discovering";
    }

signals:
    void found(QString addr);

private:
    void wasFound(QString addr)
    {
        qDebug() << "was found";
        emit found(addr);
    }

    std::unique_ptr<IppDiscovery> _ippDiscovery;
};

class QIppDiscovery : public  QStringListModel
{
    Q_OBJECT
public:
    static QIppDiscovery* instance()
    {
        static QIppDiscovery qIppDiscovery;
        return &qIppDiscovery;
    }

    Q_PROPERTY(QStringList favourites MEMBER _favourites NOTIFY favouritesChanged)

    Q_INVOKABLE void discover(bool clean=false);
    Q_INVOKABLE void reset();

signals:
    void favouritesChanged();

public slots:
    void updateFavourites();
    void insert(QString addr);

protected:
private:

    QIppDiscovery();
    ~QIppDiscovery();
    QIppDiscovery(const QIppDiscovery &);
    QIppDiscovery& operator=(const QIppDiscovery &);

    QStringList _favourites;

    QIppDiscoveryWorker _worker;
    QThread _workerThread;

};

#endif // QIPPDISCOVERY_H
