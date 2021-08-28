#ifndef CURLIODEVICE_H
#define CURLIODEVICE_H

#include <QUrl>
#include <QThread>
#include <QSemaphore>
#include <QMetaMethod>
#include <curl/curl.h>
#include <QDebug>
#include "curlworker.h"

class CurlIODevice : public QObject
{
    Q_OBJECT
public:
    CurlIODevice(QUrl addr);
    ~CurlIODevice();

    template<typename Class, typename Callback>
    bool setFinishedCallback(const Class* receiverObject, Callback cb)
    {
        connect(_performer, &CurlWorker::done, receiverObject, cb);
        return true;
    }

    void write(const char *data, size_t size);
    size_t requestWrite(char* dest, size_t size);

public slots:
    void done(CURLcode);

private:
    QUrl _addr;
    bool _open;

    CurlWorker* _performer;

    QSemaphore _canWrite;
    QSemaphore _canRead;
    bool _reading;
    bool _done;

    char* _dest;
    size_t _size;
    size_t _offset;

};

#endif // CURLIODEVICE_H
