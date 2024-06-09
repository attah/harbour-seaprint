#include "qippdiscovery.h"
#include "qippprinter.h"
#include "settings.h"

QIppDiscovery::QIppDiscovery()
: QStringListModel()
{
    _worker.moveToThread(&_workerThread);
    connect(this, &QIppDiscovery::favouritesChanged, this, &QIppDiscovery::updateFavourites);
    connect(&_worker, &QIppDiscoveryWorker::found, this, &QIppDiscovery::insert);
    _workerThread.start();
    discover();
}

QIppDiscovery::~QIppDiscovery()
{
    qDebug() << "destroy";
}

void QIppDiscovery::discover(bool clean)
{
    qDebug() << "discovering";
    QMetaObject::invokeMethod(&_worker, "discover", Qt::QueuedConnection, Q_ARG(bool, clean));
}

void QIppDiscovery::reset()
{
    setStringList({});
    discover(true);
}

void QIppDiscovery::updateFavourites()
{
    setStringList(_favourites);
    discover(true);
}

void QIppDiscovery::insert(QString addr)
{
    // TODO uniq and andle favourites
    qDebug() << "XXXXXXXXXXXXXX" << addr;
    if(!this->stringList().contains(addr))
    {
        if(this->insertRow(this->rowCount()))
        {
            QModelIndex index = this->index(this->rowCount() - 1, 0);
            this->setData(index, addr);
        }
    }
}
