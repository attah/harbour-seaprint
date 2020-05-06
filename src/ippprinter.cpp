#include "ippprinter.h"
#include <seaprint_version.h>
#include "mimer.h"

IppPrinter::IppPrinter()
{
    _nam = new QNetworkAccessManager(this);
    _print_nam = new QNetworkAccessManager(this);
    _jobs_nam = new QNetworkAccessManager(this);
    _job_cancel_nam = new QNetworkAccessManager(this);

    connect(_nam, SIGNAL(finished(QNetworkReply*)),this, SLOT(getPrinterAttributesFinished(QNetworkReply*)));
    connect(_nam, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(ignoreKnownSslErrors(QNetworkReply*, const QList<QSslError>&)));

    connect(_print_nam, SIGNAL(finished(QNetworkReply*)),this, SLOT(printRequestFinished(QNetworkReply*)));
    connect(_print_nam, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(ignoreKnownSslErrors(QNetworkReply*, const QList<QSslError>&)));

    connect(_jobs_nam, SIGNAL(finished(QNetworkReply*)),this, SLOT(getJobsRequestFinished(QNetworkReply*)));
    connect(_jobs_nam, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(ignoreKnownSslErrors(QNetworkReply*, const QList<QSslError>&)));

    connect(_job_cancel_nam, SIGNAL(finished(QNetworkReply*)),this, SLOT(cancelJobFinished(QNetworkReply*)));
    connect(_job_cancel_nam, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(ignoreKnownSslErrors(QNetworkReply*, const QList<QSslError>&)));

    QObject::connect(this, &IppPrinter::urlChanged, this, &IppPrinter::onUrlChanged);
    qRegisterMetaType<QTemporaryFile*>("QTemporaryFile*");

    _worker = new ConvertWorker;
    _worker->moveToThread(&_workerThread);

    connect(&_workerThread, &QThread::finished, _worker, &QObject::deleteLater);

    connect(this, &IppPrinter::doConvertPdf, _worker, &ConvertWorker::convertPdf);
    connect(_worker, &ConvertWorker::done, this, &IppPrinter::convertDone);
    connect(_worker, &ConvertWorker::failed, this, &IppPrinter::convertFailed);

    _workerThread.start();
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
        {"printer-uri",                 QJsonObject {{"tag", IppMsg::Uri},                 {"value", _url.toString()}}},
        {"requesting-user-name",        QJsonObject {{"tag", IppMsg::NameWithoutLanguage}, {"value", "nemo"}}},
    };
    return o;
}

void IppPrinter::setUrl(QString url_s)
{
    QUrl url = QUrl(url_s);

    qDebug() << url.scheme();

    if(url.scheme() != "ipp" /* or ipps */)
    {
        //if https -> ipps, else:
        if(url.scheme() == "")
        {
            url = QUrl("ipp://"+url_s); // Why isn't setScheme working?
        }
        else if (url.scheme() == "http") {
            url.setScheme("ipp");
        }
        else {
            url = QUrl();
        }
    }

    qDebug() << url_s << url;

    if(url != _url)
    {
        _url = url;
        emit urlChanged();
    }
}

void IppPrinter::onUrlChanged()
{
    refresh();
}

void IppPrinter::refresh() {
    _attrs = QJsonObject();
    emit attrsChanged();

    QNetworkRequest request;

    request.setUrl(httpUrl());
//    request.setRawHeader("User-Agent", "MyOwnBrowser 1.0");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/ipp");
    request.setHeader(QNetworkRequest::UserAgentHeader, "SeaPrint "SEAPRINT_VERSION);

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
    emit jobFinished(status);
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
    bool status = false;
    if(reply->error()  == QNetworkReply::NoError)
    {
        try {
            IppMsg resp(reply);
            qDebug() << resp.getStatus() << resp.getOpAttrs() << resp.getJobAttrs();
            status = resp.getStatus() <= 0xff;
        }
        catch(std::exception e)
        {
            qDebug() << e.what();
        }
    }
    emit cancelStatus(status);
    getJobs();
}



void IppPrinter::ignoreKnownSslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    QList<QSslError> IgnoredSslErrors = {QSslError::NoError,
                                         QSslError::SelfSignedCertificate,
                                         QSslError::HostNameMismatch,
                                         QSslError::UnableToGetLocalIssuerCertificate,
                                         QSslError::UnableToVerifyFirstCertificate
                                         };

    qDebug() << errors;
    for (QList<QSslError>::const_iterator it = errors.constBegin(); it != errors.constEnd(); it++) {
        if(!IgnoredSslErrors.contains(it->error())) {
            qDebug() << "Bad error: " << int(it->error()) <<  it->error();
            return;
        }
    }
    // For whatever reason, it doesn't work to pass IgnoredSslErrors here
    reply->ignoreSslErrors(errors);
}

void IppPrinter::convertDone(QNetworkRequest request, QTemporaryFile* data)
{
    connect(_print_nam, SIGNAL(finished(QNetworkReply*)), data, SLOT(deleteLater()));
    data->open();

    setBusyMessage("Transferring");

    _print_nam->post(request, data);

}

void IppPrinter::convertFailed()
{
    _jobAttrs = QJsonObject();
    _jobAttrs.insert("job-state-message", QJsonObject {{"tag", IppMsg::TextWithoutLanguage}, {"value", "Internal error"}});
    emit jobAttrsChanged();
    emit jobFinished(false);
}

void IppPrinter::print(QJsonObject attrs, QString filename){
    qDebug() << "printing" << filename << attrs;

    QFile file(filename);
    bool file_ok = file.open(QIODevice::ReadOnly);
    if(!file_ok)
    {
        emit jobFinished(false);
        return;
    }

    QFileInfo fileinfo(file);

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

    QNetworkRequest request;

    request.setUrl(httpUrl());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/ipp");
    request.setHeader(QNetworkRequest::UserAgentHeader, "SeaPrint "SEAPRINT_VERSION);


    Mimer* mimer = Mimer::instance();
    QString mimeType = mimer->get_type(filename);
    ConvertFrom from = NotConvertable;
    ConvertTarget target = NoConvert;

    if(mimeType == "application/pdf")
    {
        from = Pdf;
    }
    else if (mimeType.contains("image")) {
//        from = Image; TODO: handle image conversions
    }

    QJsonArray supportedMimeTypes = _attrs["document-format-supported"].toObject()["value"].toArray();

    qDebug() << supportedMimeTypes << supportedMimeTypes.contains(mimeType);

    if(from == Pdf /*&& !supportedMimeTypes.contains("application/pdf")*/)
    {
        if(supportedMimeTypes.contains("image/pwg-raster"))
        {
            target = PwgConvert;
        }
        else if (supportedMimeTypes.contains("image/urf"))
        {
            target = UrfConvert;
        }
    }

    quint32 HwResX = 300;
    quint32 HwResY = 300;

    if(attrs.contains("printer-resolution")) {
        // TODO: check that units == 3, aka dpi
        HwResX = attrs["printer-resolution"].toObject()["value"].toObject()["x"].toInt();
        HwResY = attrs["printer-resolution"].toObject()["value"].toObject()["y"].toInt();
    }
    else
    {
        HwResX = _attrs["printer-resolution-default"].toObject()["value"].toObject()["x"].toInt();
        HwResY = _attrs["printer-resolution-default"].toObject()["value"].toObject()["y"].toInt();
    }

    if(from == Pdf && target != NoConvert)
    {
        file.close();
        QTemporaryFile* tempfile = new QTemporaryFile();
        tempfile->open();
        tempfile->write(contents);
        qDebug() << tempfile->fileName();
        tempfile->close();

        setBusyMessage("Converting");

        emit doConvertPdf(request, filename, target==UrfConvert, HwResX, HwResY, tempfile);
    }
    else
    {
        QByteArray filedata = file.readAll();
        contents = contents.append(filedata);
        file.close();

        setBusyMessage("Transferring");

        _print_nam->post(request, contents);
    }

    return;
}

bool IppPrinter::getJobs() {

    qDebug() << "getting jobs";

    QJsonObject o = opAttrs();
    o.insert("requested-attributes", QJsonObject {{"tag", IppMsg::Keyword}, {"value", "all"}});

    IppMsg job = IppMsg(o, QJsonObject());

    QNetworkRequest request;

    QByteArray contents = job.encode(IppMsg::GetJobs);

    request.setUrl(httpUrl());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/ipp");
    request.setHeader(QNetworkRequest::UserAgentHeader, "SeaPrint "SEAPRINT_VERSION);

    _jobs_nam->post(request, contents);

    return true;
}

bool IppPrinter::cancelJob(qint32 jobId) {

    qDebug() << "cancelling jobs";

    QJsonObject o = opAttrs();
    o.insert("job-id", QJsonObject {{"tag", IppMsg::Integer}, {"value", jobId}});

    IppMsg job = IppMsg(o, QJsonObject());

    QNetworkRequest request;

    QByteArray contents = job.encode(IppMsg::CancelJob);

    request.setUrl(httpUrl());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/ipp");
    request.setHeader(QNetworkRequest::UserAgentHeader, "SeaPrint "SEAPRINT_VERSION);

    _job_cancel_nam->post(request, contents);

    return true;
}

QUrl IppPrinter::httpUrl() {
    QUrl url = _url;
    url.setScheme("http");
    if(url.port() == -1) {
        url.setPort(631);
    }
    return url;
}

void IppPrinter::setBusyMessage(QString msg)
{
    _busyMessage = msg;
    emit busyMessageChanged();
}
