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

    void resolve(QUrl& url);

signals:
    void favouritesChanged();
    void ignoreSslErrorsChanged();

public slots:
    void readPendingDatagrams();
    void cleanUpdate();
    void update();

protected:
private:
    static IppDiscovery* m_Instance;

    quint16 _transactionid;

    IppDiscovery();
    ~IppDiscovery();
    IppDiscovery(const IppDiscovery &);
    IppDiscovery& operator=(const IppDiscovery &);

    void sendQuery(quint16 qtype, QStringList addr);

    void sendQuery(quint16 qtype, QStringList prefixes, QStringList suffixes);

    void updateAndQueryPtrs(QStringList& ptrs, QStringList new_ptrs);


    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

    QStringList _ipp;
    QStringList _ipps;
    QMap<QString,QString> _rps;
    QMap<QString,quint16> _ports;
    QMap<QString,QString> _targets;

    QMultiMap<QString,QString> _AAs;
    QMultiMap<QString,QString> _AAAAs;

    QMap<QString, QTime> _outstandingQueries;

    QStringList _favourites;
    QUdpSocket* socket;
};

#endif // IPPDISCOVERY_H
