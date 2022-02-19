#include "ippdiscovery.h"
#include "ippprinter.h"
#include "settings.h"

#define A 1
#define PTR 12
#define TXT 16
#define AAAA 28
#define SRV 33

#define ALL 255 //for querying

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

IppDiscovery::IppDiscovery() : QStringListModel()
{
    socket = new QUdpSocket(this);
    connect(socket, &QUdpSocket::readyRead, this, &IppDiscovery::readPendingDatagrams);
    connect(this, &IppDiscovery::favouritesChanged, this, &IppDiscovery::cleanUpdate);
    _transactionid = 0;
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
    sendQuery(PTR, {"_ipp._tcp.local", "_ipps._tcp.local"});
}

void IppDiscovery::reset() {
    _ipp.clear();
    _rps.clear();
    _ports.clear();
    _targets.clear();

    _AAs.clear();
    _AAAAs.clear();

    _outstandingQueries.clear();

    setStringList({});
    discover();
}


void IppDiscovery::sendQuery(quint16 qtype, QStringList addrs) {
    addrs.removeDuplicates();

    QTime now = QTime::currentTime();
    QTime aWhileAgo = now.addSecs(-1);

    foreach(QString oq, _outstandingQueries.keys())
    {
        if(_outstandingQueries[oq] < aWhileAgo)
        { // Housekeeping for _outstandingQueries
            _outstandingQueries.remove(oq);
        }
        else if(addrs.contains(oq))
        { // we recently asked about this, remove it
            addrs.removeOne(oq);
        }
    }

    if(addrs.empty())
    {
        qDebug() << "nothing to do";
        return;
    }

    qDebug() << "discovering" << qtype << addrs;

    Bytestream query;
    QMap<QString, quint16> suffixPositions;

    quint16 flags = 0;
    quint16 questions = addrs.length();

    query << _transactionid++ << flags << questions << (quint16)0 << (quint16)0 << (quint16)0;

    foreach(QString addr, addrs)
    {
        _outstandingQueries.insert(addr, now);

        QStringList addrParts = addr.split(".");
        QString addrPart, restAddr;
        while(!addrParts.isEmpty())
        {
            restAddr = addrParts.join(".");
            if(suffixPositions.contains(restAddr))
            {
                query << (quint16)(0xc000 | (0x0fff & suffixPositions[restAddr]));
                break;
            }
            else
            {
                // We are putting in at least one part of the address, remember where that was
                suffixPositions.insert(restAddr, query.size());
                addrPart = addrParts.takeFirst();
                query << (quint8)addrPart.size() << addrPart.toStdString();
            }
        }
        if(addrParts.isEmpty())
        {
            // Whole addr was put in without c-pointers, 0-terminate it
            query << (quint8)0;
        }

        query << qtype << (quint16)0x0001;

    }

    QByteArray bytes((char*)(query.raw()), query.size());
    socket->writeDatagram(bytes, QHostAddress("224.0.0.251"), 5353);

}

QString make_addr(QString proto, int defaultPort, quint16 port, QString ip, QString rp)
{
    QString maybePort = port != defaultPort ? ":"+QString::number(port) : "";
    QString addr = proto+"://"+ip+maybePort+"/"+rp;
    return addr;
}

QString make_ipp_addr(quint16 port, QString ip, QString rp)
{
    return make_addr("ipp", 631, port, ip, rp);
}

QString make_ipps_addr(quint16 port, QString ip, QString rp)
{
    return make_addr("ipps", 443, port, ip, rp);
}

void IppDiscovery::cleanUpdate()
{
    setStringList(_favourites);
    update();
}

void IppDiscovery::update()
{
    QStringList found;
    QList<QPair<QString,QString>> ippsIpRps;
    QString target, rp;

    foreach(QString it, _ipps)
    {
        quint16 port = _ports[it];
        target = _targets[it];
        rp = _rps[it];

        for(QMultiMap<QString,QString>::Iterator ait = _AAs.begin(); ait != _AAs.end(); ait++)
        {
            if(ait.key() == target)
            {
                QString ip = ait.value();
                QString addr = make_ipps_addr(port, ip, rp);
                if(!found.contains(addr))
                {
                    ippsIpRps.append({ip, rp});
                    found.append(addr);
                }
            }
        }
    }

    foreach(QString it, _ipp)
    {
        quint16 port = _ports[it];
        target = _targets[it];
        rp = _rps[it];

        for(QMultiMap<QString,QString>::Iterator ait = _AAs.begin(); ait != _AAs.end(); ait++)
        {
            if(ait.key() == target)
            {
                QString ip = ait.value();
                QString addr = make_ipp_addr(port, ip, rp);

                                         // IP+RP assumed unique, don't add ipp version of the printer if ipps already added
                if(!found.contains(addr) && !ippsIpRps.contains({ip, rp}))
                {
                    found.append(addr);
                }
            }
        }
    }

    found.sort(Qt::CaseInsensitive);

    // Counting on that _ipp duplicates doesn't resolve fully any erlier than their _ipps counterpart

    // TODO?: replace this with some logica that can bpoth add and remove
    // and it can consider _favourites, so we can drop cleanUpdate
    foreach(QString f, found)
    {
        if(!this->stringList().contains(f))
        {
            if(this->insertRow(this->rowCount()))
            {
                QModelIndex index = this->index(this->rowCount() - 1, 0);
                this->setData(index, f);
            }
        }
    }
}

void IppDiscovery::updateAndQueryPtrs(QStringList& ptrs, QStringList new_ptrs)
{
    new_ptrs.removeDuplicates();
    foreach(QString ptr, new_ptrs)
    {
        if(!ptrs.contains(ptr))
        {
            ptrs.append(ptr);
        }
        // If pointer does not resolve to a target or is missing information, query about it
        if(!_targets.contains(ptr) || !_ports.contains(ptr) || !_rps.contains(ptr))
        {  // if the PTR doesn't already resolve, ask for everything about it
            sendQuery(ALL, {ptr});
        }
    }
}

void IppDiscovery::readPendingDatagrams()
{
    while (socket->hasPendingDatagrams()) {

        size_t size = socket->pendingDatagramSize();
        Bytestream resp(size);
        QHostAddress sender;
        quint16 senderPort;

        QStringList new_ipp_ptrs;
        QStringList new_ipps_ptrs;
        QStringList new_targets;

        QString qaddr, aaddr, tmpname, target;

        socket->readDatagram((char*)(resp.raw()), size, &sender, &senderPort);
        sender = QHostAddress(sender.toIPv4Address());

        quint16 transactionid, flags, questions, answerRRs, authRRs, addRRs;

        try {

            resp >> transactionid >> flags >> questions >> answerRRs >> authRRs >> addRRs;

            for(quint16 i = 0; i < questions; i++)
            {
                quint16 qtype, qflags;
                qaddr = get_addr(resp).join('.');
                resp >> qtype >> qflags;
            }

            for(quint16 i = 0; i < answerRRs; i++)
            {
                quint16 atype, aflags, len;
                quint32 ttl;

                aaddr = get_addr(resp).join('.');
                resp >> atype >> aflags >> ttl >> len;

                quint16 pos_before = resp.pos();
                if (atype == PTR)
                {
                    tmpname = get_addr(resp).join(".");
                    if(aaddr.endsWith("_ipp._tcp.local"))
                    {
                        new_ipp_ptrs.append(tmpname);
                    }
                    else if(aaddr.endsWith("_ipps._tcp.local"))
                    {
                        new_ipps_ptrs.append(tmpname);
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
                    target = get_addr(resp).join(".");
                    _ports[aaddr] = port;
                    _targets[aaddr] = target;
                    if(!new_targets.contains(target))
                    {
                        new_targets.append(target);
                    }
                }
                else if(atype == A)
                {
                    quint32 addr;
                    resp >> addr;
                    QHostAddress haddr(addr);
                    if(!_AAs.contains(aaddr, haddr.toString()))
                    {
                        _AAs.insert(aaddr, haddr.toString());
                    }
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
        qDebug() << "new ipps ptrs" << new_ipps_ptrs;
        qDebug() << "ipp ptrs" << _ipp;
        qDebug() << "ipp ptrs" << _ipps;
        qDebug() << "rps" << _rps;
        qDebug() << "ports" << _ports;
        qDebug() << "new targets" << new_targets;
        qDebug() << "targets" << _targets;
        qDebug() << "AAs" << _AAs;
        qDebug() << "AAAAs" << _AAAAs;

        // These will send one query per unique new ptr.
        // some responders doesn't give TXT records for more than one thing at at time :(
        updateAndQueryPtrs(_ipp, new_ipp_ptrs);
        updateAndQueryPtrs(_ipps, new_ipps_ptrs);

        QStringList unresolvedAddrs;

        foreach(QString t, new_targets)
        {
            // If target does not resolve to an address, query about it
            if(!_AAs.contains(t))
            {
                unresolvedAddrs.append(t);
            }
        }

        if(!unresolvedAddrs.empty())
        {
            sendQuery(A, unresolvedAddrs);
        }

    }
    this->update();

}

void IppDiscovery::resolve(QUrl& url)
{
    QString host = url.host();

    if(host.endsWith("."))
    {
        host.chop(1);
    }

    // TODO IPv6
    if(_AAs.contains(host))
    {   // TODO: retry potential other IPs
        url.setHost(_AAs.value(host));
    }
}
