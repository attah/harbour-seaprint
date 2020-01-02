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
    Q_INVOKABLE void reset();

signals:
    void favouritesChanged();

public slots:
    void readPendingDatagrams();
protected:
private:
    void sendQuery(quint16 qtype, QStringList addr);

    void update();

    QStringList _ipp;
    QMap<QString,QString> _rps;
    QMap<QString,quint16> _ports;
    QMap<QString,QString> _targets;

    QMultiMap<QString,QString> _AAs;
    QMultiMap<QString,QString> _AAAAs;

    QStringList _favourites;
    QUdpSocket* socket;
};

#endif // IPPDISCOVERY_H
