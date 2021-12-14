#include "curlworker.h"
#include "curlrequester.h"
#include "settings.h"
#include <seaprint_version.h>

Q_DECLARE_METATYPE(CURLcode)

static size_t trampoline(char* dest, size_t size, size_t nmemb, void* userp)
{
    CurlRequester* cid = (CurlRequester*)userp;
    return cid->requestWrite(dest, size*nmemb);
}

CurlWorker::CurlWorker(QUrl addr, void* parent)
{
    qRegisterMetaType<CURLcode>();

    _curl = curl_easy_init();
//    if(!curl)
//        return false;

    curl_easy_setopt(_curl, CURLOPT_URL, addr.toString().toStdString().c_str());
    curl_easy_setopt(_curl, CURLOPT_POST, 1L);
    curl_easy_setopt(_curl, CURLOPT_READFUNCTION, trampoline);
    curl_easy_setopt(_curl, CURLOPT_READDATA, parent);
//    curl_easy_setopt(_curl, CURLOPT_VERBOSE, 1L);
    if(Settings::instance()->ignoreSslErrors())
    {
        curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYSTATUS, 0L);
    }

    _opts = NULL;

    _opts = curl_slist_append(_opts, "Expect:");
    _opts = curl_slist_append(_opts, "Transfer-Encoding: chunked");
    _opts = curl_slist_append(_opts, "Content-Type: application/ipp");
    _opts = curl_slist_append(_opts, "User-Agent: SeaPrint " SEAPRINT_VERSION);
    _opts = curl_slist_append(_opts, "Accept-Encoding: identity");

    curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, _opts);

}

CurlWorker::~CurlWorker()
{
    curl_slist_free_all(_opts);
    curl_easy_cleanup(_curl);
}

void CurlWorker::run(){
    QByteArray buf;
    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &buf);
    curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, write_callback);

    CURLcode res = curl_easy_perform(_curl);
    if(res != CURLE_OK)
        qDebug() <<  "curl_easy_perform() failed: " << curl_easy_strerror(res);

    emit done(res, buf);
}
