#ifndef IPPDISCOVERY_H
#define IPPDISCOVERY_H
#include <QStringListModel>
#include <QQuickImageProvider>
#include <QUdpSocket>
#include <QMutex>
#include <QImageReader>
#include <QtNetwork>
#include <QNetworkAccessManager>
#include <QEventLoop>
#include "bytestream.h"

class IppDiscovery : public  QStringListModel, public QQuickImageProvider
{
    Q_OBJECT
public:
    static IppDiscovery* instance();

    Q_PROPERTY(QStringList favourites MEMBER _favourites NOTIFY favouritesChanged)
    Q_INVOKABLE void discover();
    Q_INVOKABLE void reset();

signals:
    void favouritesChanged();

public slots:
    void readPendingDatagrams();
    void update();
    void ignoreKnownSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);

protected:
private:
    static IppDiscovery* m_Instance;

    IppDiscovery();
    ~IppDiscovery();
    IppDiscovery(const IppDiscovery &);
    IppDiscovery& operator=(const IppDiscovery &);

    void sendQuery(quint16 qtype, QStringList addr);

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

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
