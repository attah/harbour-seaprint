#ifndef CURLWORKER_H
#define CURLWORKER_H

#include <QThread>
#include <QUrl>
#include <curl/curl.h>
#include <QtDebug>
#include <bytestream.h>

class CurlWorker : public QThread
{
    Q_OBJECT
public:
    CurlWorker(QUrl addr, void* parent);
    ~CurlWorker();

    void run() override;

    static size_t write_callback(char *ptr, size_t size, size_t nmemb, void* userdata)
    {
        size_t bytes_to_write = size*nmemb;
        ((Bytestream*)userdata)->putBytes(ptr, bytes_to_write);
        return bytes_to_write;
    }
signals:
    void done(CURLcode, Bytestream);

private:
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

    CurlWorker();
    CURL* _curl = nullptr;
    struct curl_slist* _opts;
};


#endif // CURLWORKER_H
