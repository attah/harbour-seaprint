#ifndef CURLREQUESTER_H
#define CURLREQUESTER_H

#include <QUrl>
#include <QThread>
#include <QSemaphore>
#include <QtConcurrent/QtConcurrent>
#include <curl/curl.h>
#include <bytestream.h>
#include <QDebug>
#include <functional>

class CurlRequester : public QObject
{
    Q_OBJECT
public:
    enum Role {
        IppRequest,
        HttpGetRequest
    };

    CurlRequester(QUrl addr, Role role = IppRequest);
    ~CurlRequester();

    bool write(const char *data, size_t size);
    size_t requestWrite(char* dest, size_t size);

    static size_t write_callback(char *ptr, size_t size, size_t nmemb, void* userdata)
    {
        size_t bytes_to_write = size*nmemb;
        ((Bytestream*)userdata)->putBytes(ptr, bytes_to_write);
        return bytes_to_write;
    }

signals:
    void done(CURLcode, Bytestream);

private:
    CurlRequester();

    // Container for the cURL global init and cleanup
    class GlobalEnv
    {
    public:
        GlobalEnv()
        {
            curl_global_init(CURL_GLOBAL_DEFAULT);
        }
        ~GlobalEnv()
        {
            curl_global_cleanup();
        }
    };
    // Must be run exactly once, thus static
    static GlobalEnv _gEnv;

    QUrl _addr;

    QSemaphore _canWrite;
    QSemaphore _canRead;
    bool _reading;
    bool _done;

    char* _dest;
    size_t _size;
    size_t _offset;

    friend class CurlWorker;

    CURL* _curl;
    struct curl_slist* _opts = NULL;

    QFuture<void> _worker;
};

#endif // CURLREQUESTER_H
