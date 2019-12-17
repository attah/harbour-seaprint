#include "ippprinter.h"

IppPrinter::IppPrinter()
{
    _nam = new QNetworkAccessManager(this);
    _print_nam = new QNetworkAccessManager(this);
    _jobs_nam = new QNetworkAccessManager(this);
    _job_cancel_nam = new QNetworkAccessManager(this);

    connect(_nam, SIGNAL(finished(QNetworkReply*)),this, SLOT(getPrinterAttributesFinished(QNetworkReply*)));
    connect(_print_nam, SIGNAL(finished(QNetworkReply*)),this, SLOT(printRequestFinished(QNetworkReply*)));
    connect(_jobs_nam, SIGNAL(finished(QNetworkReply*)),this, SLOT(getJobsRequestFinished(QNetworkReply*)));
    connect(_job_cancel_nam, SIGNAL(finished(QNetworkReply*)),this, SLOT(cancelJobFinished(QNetworkReply*)));
    QObject::connect(this, &IppPrinter::urlChanged, this, &IppPrinter::onUrlChanged);
}

IppPrinter::~IppPrinter() {
    delete _nam;
    delete _print_nam;
    delete _jobs_nam;
    delete _job_cancel_nam;
}

QJsonObject IppPrinter::opAttrs() {
    QJsonObject o
    {
        {"attributes-charset",          QJsonObject {{"tag", IppMsg::Charset},             {"value", "utf-8"}}},
        {"attributes-natural-language", QJsonObject {{"tag", IppMsg::NaturalLanguage},     {"value", "en-us"}}},
        {"printer-uri",                 QJsonObject {{"tag", IppMsg::Uri},                 {"value", "ipp://"+_url}}},
        {"requesting-user-name",        QJsonObject {{"tag", IppMsg::NameWithoutLanguage}, {"value", "nemo"}}},
    };
    return o;
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

    QJsonObject o = opAttrs();
    IppMsg msg = IppMsg(o);
    _nam->post(request, msg.encode(IppMsg::GetPrinterAttrs));

}

void IppPrinter::getPrinterAttributesFinished(QNetworkReply *reply)
{
    qDebug() << reply->error() << reply->errorString() << reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    _attrs = QJsonObject();
    if(reply->error()  == QNetworkReply::NoError)
    {
        try {
            IppMsg resp(reply);
            qDebug() << resp.getStatus() << resp.getOpAttrs() << resp.getPrinterAttrs();
            _attrs = resp.getPrinterAttrs();
        }
        catch(std::exception e)
        {
            qDebug() << e.what();
        }
    }
    emit attrsChanged();
}

void IppPrinter::printRequestFinished(QNetworkReply *reply)
{
    _jobAttrs = QJsonObject();
    bool status = false;
    if(reply->error()  == QNetworkReply::NoError)
    {
        try {
            IppMsg resp(reply);
            qDebug() << resp.getStatus() << resp.getOpAttrs() << resp.getJobAttrs();
            _jobAttrs = resp.getJobAttrs()[0].toObject();
            status = resp.getStatus() <= 0xff;
        }
        catch(std::exception e)
        {
            qDebug() << e.what();
        }
    }
    else {
        _jobAttrs.insert("job-state-message", QJsonObject {{"tag", IppMsg::TextWithoutLanguage}, {"value", "Network error"}});
    }
    emit jobAttrsChanged();
    emit jobAttrsFinished(status);
}

void IppPrinter::getJobsRequestFinished(QNetworkReply *reply)
{
    if(reply->error()  == QNetworkReply::NoError)
    {
        try {
            IppMsg resp(reply);
            qDebug() << resp.getStatus() << resp.getOpAttrs() << resp.getJobAttrs();
            _jobs = resp.getJobAttrs();
            emit jobsChanged();
        }
        catch(std::exception e)
        {
            qDebug() << e.what();
        }
    }
}


void IppPrinter::cancelJobFinished(QNetworkReply *reply)
{
    if(reply->error()  == QNetworkReply::NoError)
    {
        try {
            IppMsg resp(reply);
            qDebug() << resp.getStatus() << resp.getOpAttrs() << resp.getJobAttrs();
        }
        catch(std::exception e)
        {
            qDebug() << e.what();
        }
    }
    getJobs();
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

    QJsonObject o = opAttrs();
    o.insert("job-name", QJsonObject {{"tag", IppMsg::NameWithoutLanguage}, {"value", fileinfo.fileName()}});


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

    _print_nam->post(request, contents);
    file.close();
    return true;
}

bool IppPrinter::getJobs() {

    qDebug() << "getting jobs";

    QJsonObject o = opAttrs();

    IppMsg job = IppMsg(o, QJsonObject());

    QNetworkRequest request;
    QUrl url("http://"+_url);
    if(url.port() == -1) {
        url.setPort(631);
    }

    QByteArray contents = job.encode(IppMsg::GetJobs);

    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/ipp");
    _jobs_nam->post(request, contents);

    return true;
}

bool IppPrinter::cancelJob(qint32 jobId) {

    qDebug() << "getting jobs";

    QJsonObject o = opAttrs();
    o.insert("job-id", QJsonObject {{"tag", IppMsg::Integer}, {"value", jobId}});

    IppMsg job = IppMsg(o, QJsonObject());

    QNetworkRequest request;
    QUrl url("http://"+_url);
    if(url.port() == -1) {
        url.setPort(631);
    }

    QByteArray contents = job.encode(IppMsg::CancelJob);

    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/ipp");
    _job_cancel_nam->post(request, contents);

    return true;
}
