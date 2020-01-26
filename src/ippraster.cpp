#include "ippraster.h"

IppRaster::IppRaster(QIODevice *underlyingDevice)
    : underlyingDevice(underlyingDevice)
{
    connect(underlyingDevice, SIGNAL(readyRead()), this, SLOT(doDataAvail()));
    connect(underlyingDevice, SIGNAL(readChannelFinished()), this, SLOT(doReadFinished()));
}

bool IppRaster::open(QIODevice::OpenMode mode)
{
    qDebug() << "open";

    bool underlyingOk;
    if (underlyingDevice->isOpen())
        underlyingOk = (underlyingDevice->openMode() == mode);
    else
        underlyingOk = underlyingDevice->open(mode);

    if (underlyingOk) {
        setOpenMode(mode);
        qDebug() << "open true";
        emit readyRead();
        return true;
    }
    qDebug() << underlyingOk << underlyingDevice->openMode() << underlyingDevice->isOpen() << mode << "borked";
    return false;
}

void IppRaster::close()
{
    qDebug() << "close";
    underlyingDevice->close();
}

bool IppRaster::seek(qint64 pos)
{
    qDebug() << "seek";

    return false;
}

bool IppRaster::atEnd() const
{
    qDebug() << "atend";
    return underlyingDevice->atEnd();
}

qint64 IppRaster::readData(char *data, qint64 maxlen)
{
    qDebug() << "read" << maxlen;
    return underlyingDevice->read(data, maxlen);
}
qint64 IppRaster::writeData(const char *data, qint64 len)
{
    return underlyingDevice->write(data, len);
}
