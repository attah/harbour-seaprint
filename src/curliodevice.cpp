#include "curliodevice.h"
#include <seaprint_version.h>
#include <algorithm>

Q_DECLARE_METATYPE(CURLcode)


static size_t trampoline(char* dest, size_t size, size_t nmemb, void* userp)
{
    qDebug() << "boingiddy!";
    CurlIODevice* cid = (CurlIODevice*)userp;
    return cid->requestWrite(dest, size*nmemb);
}

CurlIODevice::CurlIODevice(QUrl addr) : _addr(addr), _canWrite(1), _canRead(), _reading(false), _done(false), _dest(nullptr), _size(0), _offset(0)
{
    qRegisterMetaType<CURLcode>();

//    if(openMode() != NotOpen && mode != WriteOnly)
//        return false;

    CURL* curl = curl_easy_init();
//    if(!curl)
//        return false;

    curl_easy_setopt(curl, CURLOPT_URL, _addr.toString().toStdString().c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, trampoline);
    curl_easy_setopt(curl, CURLOPT_READDATA, this);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYSTATUS, 0L);

    //TODO: free this list
    struct curl_slist *opts = NULL;

    opts = curl_slist_append(opts, "Transfer-Encoding: chunked");
    opts = curl_slist_append(opts, "Content-Type: application/ipp");
    opts = curl_slist_append(opts, "User-Agent: SeaPrint " SEAPRINT_VERSION);
    opts = curl_slist_append(opts, "Accept-Encoding: identity");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, opts);

    _performer = new CurlWorker(curl);
    connect(_performer, &CurlWorker::done, this, &CurlIODevice::done);
    connect(_performer, &CurlWorker::finished, _performer, &CurlWorker::deleteLater);
    _performer->start();

    qDebug() << "worker started";

}

CurlIODevice::~CurlIODevice()
{
    qDebug() << "destroying";

    _canWrite.acquire();
    _done = true;
    _canRead.release();
    _performer->wait();

    if(_dest != nullptr)
    {
        delete[] _dest;
    }
    qDebug() << "done destroying";

}

void CurlIODevice::write(const char *data, size_t size)
{
    qDebug() << "write " << size;
    _canWrite.acquire();

    if(_dest != nullptr)
    {
        delete[] _dest;
    }
    _dest = new char[size];
    memcpy(_dest, data, size);
    _size = size;
    _offset = 0;

    _canRead.release();

}

size_t CurlIODevice::requestWrite(char* dest, size_t size)
{
    qDebug() << "requestWrite " << size;
    if(!_reading)
    {
        _canRead.acquire();
        if(_done) // Can only have been set by write() - only relevant to check if strating to write
        {
            return 0;
        }
        _reading = true;
    }

    size_t remaining = _size - _offset;

    qDebug() << "requestWrite canwrite";

    size_t actualSize = std::min(size, remaining);
    memcpy(dest, (_dest+_offset), actualSize);
    _offset += actualSize;

    remaining = _size - _offset;
    if(remaining == 0)
    {
        _reading = false;
        _canWrite.release();
    }

    qDebug() << "requestWrite returns " << _size;
    return actualSize;
}

void CurlIODevice::done(CURLcode)
{

}
