#include "curlrequester.h"
#include <algorithm>
#include "settings.h"

static size_t trampoline(char* dest, size_t size, size_t nmemb, void* userp)
{
    CurlRequester* cid = (CurlRequester*)userp;
    return cid->requestWrite(dest, size*nmemb);
}

CurlRequester::CurlRequester(QUrl addr, Role role)
    : _addr(addr), _canWrite(1), _canRead(), _reading(false), _done(false), _dest(nullptr), _size(0), _offset(0), _curl(curl_easy_init())
{

    curl_easy_setopt(_curl, CURLOPT_URL, addr.toString().toStdString().c_str());

    bool verbose = QLoggingCategory::defaultCategory()->isDebugEnabled();

    curl_easy_setopt(_curl, CURLOPT_VERBOSE, verbose);
    if(Settings::instance()->ignoreSslErrors())
    {
        curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYSTATUS, 0L);
    }

    _opts = NULL;
    _opts = curl_slist_append(_opts, "User-Agent: SeaPrint " SEAPRINT_VERSION);


    switch (role) {
        case IppRequest:
        {
            curl_easy_setopt(_curl, CURLOPT_POST, 1L);
            curl_easy_setopt(_curl, CURLOPT_READFUNCTION, trampoline);
            curl_easy_setopt(_curl, CURLOPT_READDATA, this);

            _opts = curl_slist_append(_opts, "Expect:");
            _opts = curl_slist_append(_opts, "Transfer-Encoding: chunked");
            _opts = curl_slist_append(_opts, "Content-Type: application/ipp");
            _opts = curl_slist_append(_opts, "Accept-Encoding: identity");
            break;
        }
        case HttpGetRequest:
        {
            curl_easy_setopt(_curl, CURLOPT_HTTPGET, 1L);
            break;
        }
    }

    curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, _opts);

    _worker = QtConcurrent::run([this](){
        Bytestream buf;
        curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &buf);
        curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, write_callback);

        CURLcode res = curl_easy_perform(_curl);
        if(res != CURLE_OK)
            qDebug() <<  "curl_easy_perform() failed: " << curl_easy_strerror(res);

        emit done(res, buf);
    });
}

CurlRequester::~CurlRequester()
{
    while(!_canWrite.tryAcquire(1, 500))
    {
        if(!_worker.isRunning())
        {
            break;
        }
    }
    _done = true;
    _canRead.release();
    _worker.waitForFinished();

    if(_dest != nullptr)
    {
        delete _dest;
    }

    curl_slist_free_all(_opts);
    curl_easy_cleanup(_curl);
}

bool CurlRequester::write(const char *data, size_t size)
{
    qDebug() << "write " << size;
    while(!_canWrite.tryAcquire(1, 500))
    {
        if(!_worker.isRunning())
        {
            return false;
        }
    }

    if(_dest != nullptr)
    {
        delete _dest;
    }
    _dest = new char[size];
    memcpy(_dest, data, size);
    _size = size;
    _offset = 0;
    _canRead.release();
    return true;
}

size_t CurlRequester::requestWrite(char* dest, size_t size)
{
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

    size_t actualSize = std::min(size, remaining);

    memcpy(dest, (_dest+_offset), actualSize);
    _offset += actualSize;

    remaining = _size - _offset;
    if(remaining == 0)
    {
        _reading = false;
        _canWrite.release();
    }
    return actualSize;
}
