#include "ippprinter.h"

IppPrinter::IppPrinter()
{
    _nam = new QNetworkAccessManager(this);
    _jnam = new QNetworkAccessManager(this);
    connect(_nam, SIGNAL(finished(QNetworkReply*)),this, SLOT(getPrinterAttributesFinished(QNetworkReply*)));
    connect(_jnam, SIGNAL(finished(QNetworkReply*)),this, SLOT(jobRequestFinished(QNetworkReply*)));
    QObject::connect(this, &IppPrinter::urlChanged, this, &IppPrinter::onUrlChanged);
}

IppPrinter::~IppPrinter() {
    delete _nam;
    delete _jnam;
}

void IppPrinter::setUrl(QString url)
{
    if(url != _url)
    {
        _url = url;
        emit urlChanged();
    }
}

void IppPrinter::onUrlChanged()
{
    _attrs = QJsonObject();
    emit attrsChanged();

    QNetworkRequest request;
    QUrl url("http://"+_url);
    qDebug() << _url << url.port();
    if(url.port() == -1) {
        url.setPort(631);
    }
    request.setUrl(url);
//    request.setRawHeader("User-Agent", "MyOwnBrowser 1.0");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/ipp");

    QJsonObject o
    {
        {"attributes-charset",          QJsonObject {{"tag", IppMsg::Charset},             {"value", "utf-8"}}},
        {"attributes-natural-language", QJsonObject {{"tag", IppMsg::NaturalLanguage},     {"value", "en-us"}}},
        {"printer-uri",                 QJsonObject {{"tag", IppMsg::Uri},                 {"value", "ipp://"+_url}}},
        {"requesting-user-name",        QJsonObject {{"tag", IppMsg::NameWithoutLanguage}, {"value", "nemo"}}}
    };
    IppMsg msg = IppMsg(o);
    _nam->post(request, msg.encode(IppMsg::GetPrinterAttrs));

}

void IppPrinter::getPrinterAttributesFinished(QNetworkReply *reply)
{
    if(reply->error()  == QNetworkReply::NoError)
    {
        try {
            IppMsg resp(reply);
            _attrs = resp.getPrinterAttrs();
            emit attrsChanged();
        }
        catch(std::exception e)
        {
            qDebug() << e.what();
        }
    }

}

void IppPrinter::jobRequestFinished(QNetworkReply *reply)
{
    if(reply->error()  == QNetworkReply::NoError)
    {
        try {
            IppMsg resp(reply);
            qDebug() << resp.getStatus() << resp.getOpAttrs() << resp.getJobAttrs();
            _jobAttrs = resp.getJobAttrs();
            emit jobAttrsChanged();
        }
        catch(std::exception e)
        {
            qDebug() << e.what();
        }
    }
}


bool IppPrinter::print(QJsonObject attrs, QString filename){
    qDebug() << "printing" << filename << attrs;

    QFile file(filename);
    bool file_ok = file.open(QIODevice::ReadOnly);
    if(!file_ok)
        return false;

    QFileInfo fileinfo(file);
    QNetworkRequest request;
    QUrl url("http://"+_url);
    if(url.port() == -1) {
        url.setPort(631);
    }
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/ipp");

    QJsonObject o
    {
        {"attributes-charset",          QJsonObject {{"tag", IppMsg::Charset},             {"value", "utf-8"}}},
        {"attributes-natural-language", QJsonObject {{"tag", IppMsg::NaturalLanguage},     {"value", "en-us"}}},
        {"printer-uri",                 QJsonObject {{"tag", IppMsg::Uri},                 {"value", "ipp://"+_url}}},
        {"requesting-user-name",        QJsonObject {{"tag", IppMsg::NameWithoutLanguage}, {"value", "nemo"}}},
        {"job-name",                    QJsonObject {{"tag", IppMsg::NameWithoutLanguage}, {"value", fileinfo.fileName()}}},
    };

    // Only include if printer supports it
//    if (filename.endsWith("pdf"))
//    {
//        attrs.insert("document-format", QJsonObject {{"tag", 73}, {"value", "application/pdf"}});
//    }
//    else if (filename.endsWith("jpg")) {
//        attrs.insert("document-format", QJsonObject {{"tag", 73}, {"value", "image/jpeg"}});
//    }

    qDebug() << "Printing job" << o << attrs;
    IppMsg job = IppMsg(o, attrs);

    QByteArray contents = job.encode(IppMsg::PrintJob);
    QByteArray filedata = file.readAll();
    contents = contents.append(filedata);

    _jnam->post(request, contents);
    file.close();
    return true;
}
