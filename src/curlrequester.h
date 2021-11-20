#ifndef CURLREQUESTER_H
#define CURLREQUESTER_H

#include <QUrl>
#include <QThread>
#include <QSemaphore>
#include <QMetaMethod>
#include <curl/curl.h>
#include <QDebug>
#include "curlworker.h"

class CurlRequester : public QObject
{
    Q_OBJECT
public:
    CurlRequester(QUrl addr);
    ~CurlRequester();

    template<typename Class, typename Callback>
    bool setFinishedCallback(const Class* receiverObject, Callback cb)
    {
        connect(&_performer, &CurlWorker::done, receiverObject, cb);
        return true;
    }

    bool write(const char *data, size_t size);
    size_t requestWrite(char* dest, size_t size);


private:
    QUrl _addr;

    QSemaphore _canWrite;
    QSemaphore _canRead;
    bool _reading;
    bool _done;

    char* _dest;
    size_t _size;
    size_t _offset;

    CurlWorker _performer;
};

#endif // CURLREQUESTER_H
