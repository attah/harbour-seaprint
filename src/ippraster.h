#ifndef IPPRASTER_H
#define IPPRASTER_H

#include <qiodevice.h>
#include <QDebug>


class IppRaster : public QIODevice
{
    Q_OBJECT

public:
    IppRaster(QIODevice *underlyingDevice);
    ~IppRaster() {qDebug() << "deleted";}

    bool open(QIODevice::OpenMode mode) override;
    void close() override;
    bool seek(qint64) override;
    bool atEnd() const override;

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

    bool isSequential() const override {return false;}
    qint64 bytesAvailable() const override {return underlyingDevice->bytesAvailable();}
    qint64 size() const override {return underlyingDevice->size();}

    bool waitForReadyRead(int msecs) {return true;}

    public slots:
    void doDataAvail() {emit readyRead();}
    void doReadFinished() {emit readChannelFinished();}


private:
    QIODevice* underlyingDevice;
};

#endif
