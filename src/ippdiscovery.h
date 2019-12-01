#ifndef IPPDISCOVERY_H
#define IPPDISCOVERY_H
#include <QStringListModel>
#include <QUdpSocket>
#include "bytestream.h"

class IppDiscovery : public  QStringListModel
{
    Q_OBJECT
public:
    IppDiscovery();
    ~IppDiscovery();
    Q_PROPERTY(QStringList favourites MEMBER _favourites NOTIFY favouritesChanged)
    Q_INVOKABLE void discover();

signals:
    void favouritesChanged();

public slots:
    void readPendingDatagrams();
    void update();
protected:
private:
    QStringList _favourites;
    QStringList _found;
    QUdpSocket* socket;
};

#endif // IPPDISCOVERY_H
