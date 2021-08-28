#include "curlworker.h"

CurlWorker::CurlWorker(CURL* curl)
{
    _curl = curl;
}


void CurlWorker::run(){
    QByteArray buf;
    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &buf);
    curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, write_callback);

    qDebug() << "performer running";
    CURLcode res = curl_easy_perform(_curl);
    if(res != CURLE_OK)
        qDebug() <<  "curl_easy_perform() failed: " << curl_easy_strerror(res);
    curl_easy_cleanup(_curl);
    curl_global_cleanup();
    qDebug() << "performer done " << buf.length();
    emit done(res, buf);
}
