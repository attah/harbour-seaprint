#include "ippdiscovery.h"
#include "ippprinter.h"
#include <seaprint_version.h>

#define A 1
#define PTR 12
#define TXT 16
#define AAAA 28
#define SRV 33

#define ALL 255 //for querying

void put_addr(Bytestream& bts, QStringList addr)
{
    for(int i = 0; i < addr.length(); i++)
    {
        QString elem = addr[i];
        bts << (quint8)elem.size() << elem.toStdString();
    }
    bts << (quint8)0;
}

QStringList get_addr(Bytestream& bts)
{
    QStringList addr;
    while(true)
    {
        if(bts.nextU8(0))
        {
            break;
        }
        else if ((bts.peekU8()&0xc0)==0xc0)
        {
            quint16 ref = bts.getU16() & 0x0fff;
            Bytestream tmp = bts;
            tmp.setPos(ref);
            addr += get_addr(tmp);
            break;
        }
        else
        {
            std::string elem;
            bts/bts.getU8() >> elem;
            addr.append(QString(elem.c_str()));
        }
    }
    return addr;
}

IppDiscovery::IppDiscovery() : QStringListModel(), QQuickImageProvider(QQuickImageProvider::Image, ForceAsynchronousImageLoading)
{
    socket = new QUdpSocket(this);
    connect(socket, SIGNAL(readyRead()),
            this, SLOT(readPendingDatagrams()));
    connect(this, SIGNAL(favouritesChanged()),
            this, SLOT(update()));
}

IppDiscovery::~IppDiscovery() {
    delete socket;
}

IppDiscovery* IppDiscovery::m_Instance = nullptr;

IppDiscovery* IppDiscovery::instance()
{
    static QMutex mutex;
    if (!m_Instance)
    {
        mutex.lock();

        if (!m_Instance)
            m_Instance = new IppDiscovery;

        mutex.unlock();
    }

    return m_Instance;
}

void IppDiscovery::discover() {
    sendQuery(PTR, {"_ipp","_tcp","local"});
}

void IppDiscovery::reset() {
    _ipp = QStringList();
    _rps = QMap<QString,QString>();
    _ports = QMap<QString,quint16>();
    _targets = QMap<QString,QString>();

    _AAs = QMultiMap<QString,QString>();
    _AAAAs = QMultiMap<QString,QString>();

    discover();
}

void IppDiscovery::sendQuery(quint16 qtype, QStringList addr) {
    qDebug() << "discovering" << qtype << addr;

    Bytestream query;
    quint16 transactionid = 0;
    quint16 flags = 0;
    quint16 questions = 1;

    query << transactionid << flags << questions << (quint16)0 << (quint16)0 << (quint16)0;
    put_addr(query, addr);
    query << qtype << (quint16)0x0001;

    QByteArray bytes((char*)(query.raw()), query.size());
    socket->writeDatagram(bytes, QHostAddress("224.0.0.251"), 5353);

}


void IppDiscovery::update()
{
    QStringList found;

    for(QStringList::Iterator it = _ipp.begin(); it != _ipp.end(); it++)
    {
        quint16 port = _ports[*it];
        QString target = _targets[*it];
        QString rp = _rps[*it];

        for(QMultiMap<QString,QString>::Iterator ait = _AAs.begin(); ait != _AAs.end(); ait++)
        {
            if(ait.key() == target)
            {
                QString ip = ait.value();
                QString maybePort = port != 631 ? ":"+QString::number(port) : "";
                QString addr = "ipp://"+ip+maybePort+"/"+rp;
                if(!found.contains(addr))
                {
                    found.append(addr);
                    found.sort(Qt::CaseInsensitive);
                }
            }
        }
    }

    qDebug() << _favourites << found;
    this->setStringList(_favourites+found);
}

void IppDiscovery::readPendingDatagrams()
{
    while (socket->hasPendingDatagrams()) {

        size_t size = socket->pendingDatagramSize();
        Bytestream resp(size);
        QHostAddress sender;
        quint16 senderPort;

        QStringList new_ipp_ptrs;
        QStringList new_targets;

        socket->readDatagram((char*)(resp.raw()), size, &sender, &senderPort);
        sender = QHostAddress(sender.toIPv4Address());

        quint16 transactionid, flags, questions, answerRRs, authRRs, addRRs;

        try {

            resp >> transactionid >> flags >> questions >> answerRRs >> authRRs >> addRRs;

            for(quint16 i = 0; i < questions; i++)
            {
                quint16 qtype, qflags;
                QString qaddr = get_addr(resp).join('.');
                resp >> qtype >> qflags;
            }

            for(quint16 i = 0; i < answerRRs; i++)
            {
                quint16 atype, aflags, len;
                quint32 ttl;

                QString aaddr = get_addr(resp).join('.');
                resp >> atype >> aflags >> ttl >> len;

                quint16 pos_before = resp.pos();
                if (atype == PTR)
                {
                    QString tmpname = get_addr(resp).join(".");
                    if(aaddr.endsWith("_ipp._tcp.local"))
                    {
                        new_ipp_ptrs.append(tmpname);
                    }
                }
                else if(atype == TXT)
                {
                    Bytestream tmp;
                    while(resp.pos() < pos_before+len)
                    {
                        resp/resp.getU8() >> tmp;
                        if(tmp >>= "rp=")
                        {
                            std::string tmprp;
                            tmp/tmp.remaining() >> tmprp;
                            _rps[aaddr] = tmprp.c_str();
                        }
                    }
                }
                else if (atype == SRV)
                {
                    quint16 prio, w, port;
                    resp >> prio >> w >> port;
                    QString target = get_addr(resp).join(".");
                    _ports[aaddr] = port;
                    _targets[aaddr] = target;
                    new_targets.append(target);
                }
                else if(atype == A)
                {
                    quint32 addr;
                    resp >> addr;
                    QHostAddress haddr(addr);
                    _AAs.insert(aaddr, haddr.toString());
                }
                else
                {
                    resp += len;
                }
                Q_ASSERT(resp.pos() == pos_before+len);

            }
        }
        catch(const std::exception& e)
        {
            qDebug() << e.what();
            return;
        }
        qDebug() << "new ipp ptrs" << new_ipp_ptrs;
        qDebug() << "ipp ptrs" << _ipp;
        qDebug() << "rps" << _rps;
        qDebug() << "ports" << _ports;
        qDebug() << "new targets" << new_targets;
        qDebug() << "targets" << _targets;
        qDebug() << "AAs" << _AAs;
        qDebug() << "AAAAs" << _AAAAs;

        for(QStringList::Iterator it = new_ipp_ptrs.begin(); it != new_ipp_ptrs.end(); it++)
        {
            if(!_ipp.contains(*it))
            {
                _ipp.append(*it);
            }
            // If pointer does not resolve to a target or is missing information, query about it
            if( !_targets.contains(*it) || !_ports.contains(*it) || !_rps.contains(*it))
            {  // if the PTR doesn't already resolve, ask for everything about it
                sendQuery(ALL, it->split('.'));
            }
        }
        for(QStringList::Iterator it = new_targets.begin(); it != new_targets.end(); it++)
        {
            // If target does not resolve to an address, query about it
            if(!_AAs.contains(*it))
            {
                sendQuery(ALL, it->split('.'));
            }
        }

    }
    this->update();

}

QImage IppDiscovery::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{   //TODO: consider caching images (doesn't appear to be needed currently)
    Q_UNUSED(requestedSize);
    qDebug() << "requesting image" << id;

    QImage img;

    QNetworkAccessManager* nam = new QNetworkAccessManager();
    QUrl url(id);
    QString host = url.host();

    if(host.endsWith("."))
    {
        host.chop(1);
    }

    qDebug() << url.host() << host << _AAs;
    // TODO IPv6
    if(_AAs.contains(host))
    {   // TODO: retry potential other IPs
        url.setHost(_AAs.value(host));
    }

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "SeaPrint " SEAPRINT_VERSION);

    connect(nam, &QNetworkAccessManager::sslErrors,
            &IppPrinter::ignoreKnownSslErrors);

    QNetworkReply* reply = nam->get(request);

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if (reply->error() == QNetworkReply::NoError)
    {
        QImageReader imageReader(reply);
        img = imageReader.read();
     }

    *size = img.size();
    delete nam;
    return img;

}
