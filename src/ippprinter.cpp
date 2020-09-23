#include "ippprinter.h"
#include <seaprint_version.h>
#include "mimer.h"
#include "papersizes.h"

IppPrinter::IppPrinter()
{
    _nam = new QNetworkAccessManager(this);
    _print_nam = new QNetworkAccessManager(this);
    _jobs_nam = new QNetworkAccessManager(this);
    _job_cancel_nam = new QNetworkAccessManager(this);

    connect(_nam, &QNetworkAccessManager::finished, this, &IppPrinter::getPrinterAttributesFinished);
    connect(_nam, &QNetworkAccessManager::sslErrors, this, &IppPrinter::ignoreKnownSslErrors);

    connect(_print_nam, &QNetworkAccessManager::finished, this, &IppPrinter::printRequestFinished);
    connect(_print_nam, &QNetworkAccessManager::sslErrors, this, &IppPrinter::ignoreKnownSslErrors);

    connect(_jobs_nam, &QNetworkAccessManager::finished,this, &IppPrinter::getJobsRequestFinished);
    connect(_jobs_nam, &QNetworkAccessManager::sslErrors, this, &IppPrinter::ignoreKnownSslErrors);

    connect(_job_cancel_nam, &QNetworkAccessManager::finished,this, &IppPrinter::cancelJobFinished);
    connect(_job_cancel_nam, &QNetworkAccessManager::sslErrors, this, &IppPrinter::ignoreKnownSslErrors);

    QObject::connect(this, &IppPrinter::urlChanged, this, &IppPrinter::onUrlChanged);
    qRegisterMetaType<QTemporaryFile*>("QTemporaryFile*");

    _worker = new ConvertWorker;
    _worker->moveToThread(&_workerThread);

    connect(&_workerThread, &QThread::finished, _worker, &QObject::deleteLater);

    connect(this, &IppPrinter::doConvertPdf, _worker, &ConvertWorker::convertPdf);
    connect(this, &IppPrinter::doConvertImage, _worker, &ConvertWorker::convertImage);
    connect(_worker, &ConvertWorker::done, this, &IppPrinter::convertDone);
    connect(_worker, &ConvertWorker::progress, this, &IppPrinter::setProgress);
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
    QString name = qgetenv("USER");
    QJsonObject o
    {
        {"attributes-charset",          QJsonObject {{"tag", IppMsg::Charset},             {"value", "utf-8"}}},
        {"attributes-natural-language", QJsonObject {{"tag", IppMsg::NaturalLanguage},     {"value", "en-us"}}},
        {"printer-uri",                 QJsonObject {{"tag", IppMsg::Uri},                 {"value", _url.toString()}}},
        {"requesting-user-name",        QJsonObject {{"tag", IppMsg::NameWithoutLanguage}, {"value", name}}},
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

    _additionalDocumentFormats = QStringList();
    emit additionalDocumentFormatsChanged();


    QNetworkRequest request;

    request.setUrl(httpUrl());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/ipp");
    request.setHeader(QNetworkRequest::UserAgentHeader, "SeaPrint " SEAPRINT_VERSION);

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
        catch(const std::exception& e)
        {
            qDebug() << e.what();
        }
    }

    if(_attrs.contains("printer-device-id"))
    {
        QJsonArray supportedMimeTypes = _attrs["document-format-supported"].toObject()["value"].toArray();
        QStringList printerDeviceId = _attrs["printer-device-id"].toObject()["value"].toString().split(";");
        for (QStringList::iterator it = printerDeviceId.begin(); it != printerDeviceId.end(); it++)
        {
            QStringList kv = it->split(":");
            if(kv.length()==2 && (kv[0]=="CMD" || kv[0]=="COMMAND SET"))
            {
                if(!supportedMimeTypes.contains("application/pdf") && kv[1].contains("PDF"))
                {
                    _additionalDocumentFormats.append("application/pdf");
                }
                if(!supportedMimeTypes.contains("application/postscript") &&
                   kv[1].contains("Postscript", Qt::CaseInsensitive))
                {
                    _additionalDocumentFormats.append("application/postscript");
                }
            }
        }
        qDebug() << "additionalDocumentFormats" << _additionalDocumentFormats;
        emit additionalDocumentFormatsChanged();
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
            if(resp.getOpAttrs().keys().contains("status-message"))
            { // Sometimes there are no response attributes at all,
              // maybe status-message from the operation attributes is somewhat useful
                _jobAttrs["status-message"] = resp.getOpAttrs()["status-message"];
            }
            status = resp.getStatus() <= 0xff;
        }
        catch(const std::exception& e)
        {
            qDebug() << e.what();
        }
    }
    else {
        _jobAttrs.insert("job-state-message", QJsonObject {{"tag", IppMsg::TextWithoutLanguage},
                                                           {"value", "Network error"}});
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
        catch(const std::exception& e)
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
        catch(const std::exception& e)
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

    QNetworkReply* reply = _print_nam->post(request, data);

    connect(reply, &QNetworkReply::uploadProgress, this, &IppPrinter::setProgress);

}

void IppPrinter::convertFailed(QString message)
{
    _jobAttrs = QJsonObject();
    _jobAttrs.insert("job-state-message", QJsonObject {{"tag", IppMsg::TextWithoutLanguage}, {"value", message}});
    emit jobAttrsChanged();
    emit jobFinished(false);
}

QString firstMatch(QJsonArray supported, QStringList wanted)
{
    for(QStringList::iterator it = wanted.begin(); it != wanted.end(); it++)
    {
        if(supported.contains(*it))
        {
            return *it;
        }
    }
    return "";
}

QString targetFormatIfAuto(QString documentFormat, QString mimeType, QJsonArray supportedMimeTypes, bool forceRaster)
{
    if(forceRaster)
    {
        return firstMatch(supportedMimeTypes, {"image/pwg-raster", "image/urf"});
    }
    else if(documentFormat == "application/octet-stream")
    {
        if(mimeType == "application/pdf")
        {
            return firstMatch(supportedMimeTypes, {"application/pdf", "application/postscript", "image/pwg-raster", "image/urf" });
        }
        else if (mimeType.contains("image"))
        {
            QStringList ImageFormatPrioList {"image/png", "image/gif", "image/pwg-raster", "image/urf", "image/jpeg"};
            if(mimeType == "image/jpeg")
            {
                // Prioritize transferring JPEG as JPEG, as it will not be transcoded
                // Normally, it is the last choice (as the others are lossless)
                ImageFormatPrioList.prepend(mimeType);
            }
            return firstMatch(supportedMimeTypes, ImageFormatPrioList);
        }
        return documentFormat;
    }
    return documentFormat;
}

void IppPrinter::print(QJsonObject attrs, QString filename,
                       bool alwaysConvert, bool removeRedundantConvertAttrs, bool alwaysUseMediaCol)
{
    qDebug() << "printing" << filename << attrs
             << alwaysConvert << removeRedundantConvertAttrs;

    _progress = "";
    emit progressChanged();

    QFile file(filename);
    bool file_ok = file.open(QIODevice::ReadOnly);
    if(!file_ok)
    {
        emit convertFailed(tr("Failed to open file"));
        return;
    }

    Mimer* mimer = Mimer::instance();
    QString mimeType = mimer->get_type(filename);


    QJsonArray supportedMimeTypes = _attrs["document-format-supported"].toObject()["value"].toArray();
    for(QStringList::iterator it = _additionalDocumentFormats.begin(); it != _additionalDocumentFormats.end(); it++)
    {
        supportedMimeTypes.append(*it);
    }

    qDebug() << supportedMimeTypes << supportedMimeTypes.contains(mimeType);

    QFileInfo fileinfo(file);

    QJsonObject o = opAttrs();
    o.insert("job-name", QJsonObject {{"tag", IppMsg::NameWithoutLanguage}, {"value", fileinfo.fileName()}});

    QString PaperSize = getAttrOrDefault(attrs, "media").toString();

    if((attrs.contains("media-col") || alwaysUseMediaCol) && attrs.contains("media"))
    {
        qDebug() << "moving media to media-col" << PaperSize;
        if(!PaperSizes.contains(PaperSize))
        {
            emit convertFailed(tr("Unknown document format dimensions"));
            return;
        }

        int x = PaperSizes[PaperSize].first*100;
        int y = PaperSizes[PaperSize].second*100;

        QJsonObject Dimensions =
            {{"tag", IppMsg::BeginCollection},
             {"value", QJsonObject { {"x-dimension", QJsonObject{{"tag", IppMsg::Integer}, {"value", x}}},
                                     {"y-dimension", QJsonObject{{"tag", IppMsg::Integer}, {"value", y}}} }
             }};

        // TODO: make a setter function
        QJsonObject MediaCol = attrs["media-col"].toObject();
        QJsonObject MediaColValue = MediaCol["value"].toObject();
        MediaColValue["media-size"] = Dimensions;
        MediaCol["value"] = MediaColValue;
        MediaCol["tag"] = IppMsg::BeginCollection;
        attrs["media-col"] = MediaCol;

        attrs.remove("media");
    }

    QString documentFormat = getAttrOrDefault(attrs, "document-format").toString();
    qDebug() << "target format:" << documentFormat << "alwaysConvert:" << alwaysConvert;

    // document-format goes in the op-attrs and not the job-attrs
    o.insert("document-format", QJsonObject {{"tag", IppMsg::MimeMediaType}, {"value", documentFormat}});
    attrs.remove("document-format");

    documentFormat = targetFormatIfAuto(documentFormat, mimeType, supportedMimeTypes, alwaysConvert);
    qDebug() << "adjusted target format:" << documentFormat;

    if(documentFormat == "" || documentFormat == "application/octet-string")
    {
        emit convertFailed(tr("Unknown document format"));
        return;
    }

    qDebug() << "Printing job" << o << attrs;

    QNetworkRequest request;

    request.setUrl(httpUrl());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/ipp");
    request.setHeader(QNetworkRequest::UserAgentHeader, "SeaPrint " SEAPRINT_VERSION);

    QJsonValue PrinterResolutionRef = getAttrOrDefault(attrs, "printer-resolution");
    quint32 HwResX = PrinterResolutionRef.toObject()["x"].toInt();
    quint32 HwResY = PrinterResolutionRef.toObject()["y"].toInt();

    if(documentFormat == "image/urf")
    { // Ensure symmetric resolution for URF
        if(HwResX < HwResY)
        {
            HwResY = HwResX;
        }
        else
        {
            HwResX = HwResY;
        }
        QJsonObject tmpObj;
        tmpObj["units"] = PrinterResolutionRef.toObject()["units"];
        tmpObj["x"] = (int)HwResX;
        tmpObj["y"] = (int)HwResY;
        attrs["printer-resolution"] = QJsonObject { {"tag", IppMsg::Resolution}, {"value", tmpObj} };
    }

    quint32 Quality = getAttrOrDefault(attrs, "print-quality").toInt();

    QString PrintColorMode = getAttrOrDefault(attrs, "print-color-mode").toString();
    quint32 Colors = PrintColorMode.contains("color") ? 3 : PrintColorMode.contains("monochrome") ? 1 : 0;
    bool pdfPageRangeAdjustNeeded = false;

    if(!PaperSizes.contains(PaperSize))
    {
        emit convertFailed(tr("Unsupported print media"));
        return;
    }

    quint32 PageRangeLow = 0;
    quint32 PageRangeHigh = 0;
    if(attrs.contains("page-ranges"))
    {
        QJsonObject PageRanges = getAttrOrDefault(attrs, "page-ranges").toObject();
        PageRangeLow = PageRanges["low"].toInt();
        PageRangeHigh = PageRanges["high"].toInt();
    }

    QString Sides = getAttrOrDefault(attrs, "sides").toString();
    if(removeRedundantConvertAttrs && (documentFormat=="image/pwg-raster" ||
                                       documentFormat=="image/urf"))
    {
        attrs.remove("sides");
        attrs.remove("print-color-mode");
        attrs.remove("page-ranges");
    }
    else if(removeRedundantConvertAttrs && documentFormat == "application/postscript")
    {
        attrs.remove("sides");
        attrs.remove("page-ranges");
    }
    else if (documentFormat == "application/pdf")
    {
        if(attrs.contains("page-ranges") && !_attrs.contains("page-ranges-supported"))
        {
            pdfPageRangeAdjustNeeded = true;
            attrs.remove("page-ranges");
        }
    }

    qDebug() << "Final op attributes:" << o;
    qDebug() << "Final job attributes:" << attrs;

    IppMsg job = mk_msg(o, attrs);
    QByteArray contents = job.encode(IppMsg::PrintJob);

    // Non-jpeg images, Postscript and PDF (when not adjusting pages locally)
    // Always convert non-jpeg images to get resizing
    // TODO: make this sane
    if((mimeType == documentFormat)
       && (documentFormat == "image/jpeg" || !mimeType.contains("image"))
       && !((documentFormat == "application/pdf") && pdfPageRangeAdjustNeeded))
    {
        QByteArray filedata = file.readAll();
        contents = contents.append(filedata);
        file.close();

        setBusyMessage("Transferring");
        QNetworkReply* reply = _print_nam->post(request, contents);
        connect(reply, &QNetworkReply::uploadProgress, this, &IppPrinter::setProgress);
    }
    else
    {
        file.close();

        QTemporaryFile* tempfile = new QTemporaryFile();
        tempfile->open();
        tempfile->write(contents);
        qDebug() << tempfile->fileName();
        tempfile->close();

        setBusyMessage("Converting");

        if(mimeType == "application/pdf")
        {
            bool TwoSided = false;
            bool Tumble = false;
            if(Sides=="two-sided-long-edge")
            {
                TwoSided = true;
            }
            else if(Sides=="two-sided-short-edge")
            {
                TwoSided = true;
                Tumble = true;
            }

            emit doConvertPdf(request, filename, tempfile, documentFormat, Colors, Quality,
                              PaperSize, HwResX, HwResY, TwoSided, Tumble, PageRangeLow, PageRangeHigh);
        }
        else if (mimeType.contains("image"))
        {
            emit doConvertImage(request, filename, tempfile, documentFormat, Colors, Quality,
                                PaperSize, HwResX, HwResY);
        }
        else
        {
            emit convertFailed(tr("Cannot convert this file format"));
            return;
        }
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
    request.setHeader(QNetworkRequest::UserAgentHeader, "SeaPrint " SEAPRINT_VERSION);

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
    request.setHeader(QNetworkRequest::UserAgentHeader, "SeaPrint " SEAPRINT_VERSION);

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

void IppPrinter::setProgress(qint64 sent, qint64 total)
{
    if(total == 0)
        return;

    _progress = QString::number(100*sent/total);
    _progress += "%";
    emit progressChanged();
}

QJsonValue IppPrinter::getAttrOrDefault(QJsonObject jobAttrs, QString name)
{
    if(jobAttrs.contains(name))
    {
        return jobAttrs[name].toObject()["value"];
    }
    else {
        return _attrs[name+"-default"].toObject()["value"];
    }
}

IppMsg IppPrinter::mk_msg(QJsonObject opAttrs, QJsonObject jobAttrs)
{
    if(_attrs.contains("ipp-versions-supported") &&
       _attrs["ipp-versions-supported"].toObject()["value"].toArray().contains("2.0"))
    {
        qDebug() << "TWO-POINT-ZERO";
        return IppMsg(opAttrs, jobAttrs, 2, 0);
    }
    return IppMsg(opAttrs, jobAttrs);
}
