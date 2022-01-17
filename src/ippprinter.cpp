#include "ippprinter.h"
#include "ippdiscovery.h"
#include "mimer.h"
#include "papersizes.h"
#include "overrider.h"
#include "settings.h"

Q_DECLARE_METATYPE(QMargins)

IppPrinter::IppPrinter() : _worker(this)
{
    QObject::connect(this, &IppPrinter::urlChanged, this, &IppPrinter::onUrlChanged);
    qRegisterMetaType<QTemporaryFile*>("QTemporaryFile*");

    _worker.moveToThread(&_workerThread);

    connect(this, &IppPrinter::doDoGetPrinterAttributes, &_worker, &PrinterWorker::getPrinterAttributes);
    connect(this, &IppPrinter::doGetJobs, &_worker, &PrinterWorker::getJobs);
    connect(this, &IppPrinter::doCancelJob, &_worker, &PrinterWorker::cancelJob);
    connect(this, &IppPrinter::doJustUpload, &_worker, &PrinterWorker::justUpload);

    connect(this, &IppPrinter::doConvertPdf, &_worker, &PrinterWorker::convertPdf);
    connect(this, &IppPrinter::doConvertImage, &_worker, &PrinterWorker::convertImage);
    connect(this, &IppPrinter::doConvertOfficeDocument, &_worker, &PrinterWorker::convertOfficeDocument);
    connect(this, &IppPrinter::doConvertPlaintext, &_worker, &PrinterWorker::convertPlaintext);

    connect(this, &IppPrinter::doGetStrings, &_worker, &PrinterWorker::getStrings);

    connect(&_worker, &PrinterWorker::progress, this, &IppPrinter::setProgress);
    connect(&_worker, &PrinterWorker::busyMessage, this, &IppPrinter::setBusyMessage);
    connect(&_worker, &PrinterWorker::failed, this, &IppPrinter::convertFailed);

    qRegisterMetaType<QMargins>();

    _workerThread.start();
}

IppPrinter::~IppPrinter() {

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

    // If not already a good scheme, try to fixup, or give an empty url
    if(url.scheme() != "ipp" && url.scheme() != "ipps" && url.scheme() != "file")
    {
        if(url.scheme() == "")
        {
            url = QUrl("ipp://"+url_s); // Why isn't setScheme working?
        }
        else if (url.scheme() == "http") {
            url.setScheme("ipp");
        }
        else if (url.scheme() == "https") {
            url.setScheme("ipps");
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

    if(_url.scheme() == "file")
    {
        _attrs = QJsonObject();

        QFile file(_url.toLocalFile());
        if(file.open(QIODevice::ReadOnly))
        {
            QJsonDocument JsonDocument = QJsonDocument::fromJson(file.readAll());

            _attrs = JsonDocument.object();
            // These won't load anyway...r
            _attrs.remove("printer-icons");
            file.close();
            Overrider::instance()->apply(_attrs);
        }
        emit attrsChanged();

        MaybeGetStrings();
        UpdateAdditionalDocumentFormats();
    }
    else
    {
        QJsonObject o = opAttrs();

        IppMsg msg = IppMsg(o);
        emit doDoGetPrinterAttributes(msg.encode(IppMsg::GetPrinterAttrs));
    }
}

void IppPrinter::MaybeGetStrings()
{
    if(_attrs.contains("printer-strings-uri") && _strings.empty())
    {
        QUrl url(_attrs["printer-strings-uri"].toObject()["value"].toString());
        IppDiscovery::instance()->resolve(url);
        emit doGetStrings(url);
    }
}

void IppPrinter::UpdateAdditionalDocumentFormats()
{
    _additionalDocumentFormats = QStringList();

    if(_attrs.contains("printer-device-id"))
    {
        QJsonArray supportedMimeTypes = _attrs["document-format-supported"].toObject()["value"].toArray();
        QStringList printerDeviceId = _attrs["printer-device-id"].toObject()["value"].toString().split(";");
        for (QStringList::iterator it = printerDeviceId.begin(); it != printerDeviceId.end(); it++)
        {
            QStringList kv = it->split(":");
            if(kv.length()==2 && (kv[0]=="CMD" || kv[0]=="COMMAND SET"))
            {
                if(!supportedMimeTypes.contains(Mimer::PDF) && kv[1].contains("PDF"))
                {
                    _additionalDocumentFormats.append(Mimer::PDF);
                }
                if(!supportedMimeTypes.contains(Mimer::Postscript) &&
                   kv[1].contains("Postscript", Qt::CaseInsensitive))
                {
                    _additionalDocumentFormats.append(Mimer::Postscript);
                }
            }
        }
        qDebug() << "additionalDocumentFormats" << _additionalDocumentFormats;
    }
    emit additionalDocumentFormatsChanged();
}

void IppPrinter::getPrinterAttributesFinished(CURLcode res, Bytestream data)
{
    qDebug() << res;
    _attrs = QJsonObject();

    if(res == CURLE_OK)
    {
        try {
            IppMsg resp(data);
            qDebug() << resp.getStatus() << resp.getOpAttrs() << resp.getPrinterAttrs();
            _attrs = resp.getPrinterAttrs();
            Overrider::instance()->apply(_attrs);

        }
        catch(const std::exception& e)
        {
            qDebug() << e.what();
        }
    }

    emit attrsChanged();

    MaybeGetStrings();
    UpdateAdditionalDocumentFormats();
}

void IppPrinter::printRequestFinished(CURLcode res, Bytestream data)
{
    _jobAttrs = QJsonObject();
    bool status = false;

    if(res == CURLE_OK)
    {
        try {
            IppMsg resp(data);
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

void IppPrinter::getJobsRequestFinished(CURLcode res, Bytestream data)
{
    if(res == CURLE_OK)
    {
        try {
            IppMsg resp(data);
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


void IppPrinter::cancelJobFinished(CURLcode res, Bytestream data)
{
    bool status = false;
    if(res == CURLE_OK)
    {
        try {
            IppMsg resp(data);
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

void IppPrinter::getStringsFinished(CURLcode res, Bytestream data)
{
    qDebug() << res << data.size();
    if(res == CURLE_OK)
    {
        QByteArray ba((char*)data.raw(), data.size());
        // "media-type.com.epson-coated" = "Epson Photo Quality Ink Jet";
        QRegularExpression re("^\\\"(.*)\\\"\\s*=\\s*\\\"(.*)\\\";");
        QList<QByteArray> bl = ba.split('\n');
        foreach(QByteArray l, bl)
        {
            QRegularExpressionMatch match = re.match(l);
            if(match.hasMatch())
            {
                _strings[match.captured(1)] = match.captured(2);
            }
        }
    }
}

void IppPrinter::ignoreSslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    bool ignore = Settings::instance()->ignoreSslErrors();
    qDebug() << reply->request().url() <<  "SSL handshake failed" << errors << ignore;
    if(ignore)
    {
        reply->ignoreSslErrors(errors);
    }
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

QString targetFormatIfAuto(QString documentFormat, QString mimeType, QJsonArray supportedMimeTypes)
{
    if(documentFormat == Mimer::OctetStream)
    {
        if(mimeType == Mimer::PDF || mimeType == Mimer::Plaintext)
        {
            return firstMatch(supportedMimeTypes, {Mimer::PDF, Mimer::Postscript, Mimer::PWG, Mimer::URF });
        }
        else if(mimeType == Mimer::Postscript)
        {
            return firstMatch(supportedMimeTypes, {Mimer::Postscript});
        }
        else if(Mimer::isOffice(mimeType))
        {
            return firstMatch(supportedMimeTypes, {Mimer::PDF, Mimer::Postscript, Mimer::PWG, Mimer::URF });
        }
        else if(Mimer::isImage(mimeType))
        {
            QStringList ImageFormatPrioList {Mimer::PNG, Mimer::PWG, Mimer::URF, Mimer::PDF, Mimer::Postscript, Mimer::JPEG};
            if(mimeType == Mimer::JPEG)
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

void IppPrinter::adjustRasterSettings(QString documentFormat, QJsonObject& jobAttrs, quint32& HwResX, quint32& HwResY,
                                      bool& BackHFlip, bool& BackVFlip)
{
    if(documentFormat != Mimer::PWG && documentFormat != Mimer::URF)
    {
        return;
    }

    //TODO? jobAttrs.remove("printer-resolution");

    if(documentFormat == Mimer::PWG)
    {
        quint32 diff = std::numeric_limits<quint32>::max();
        quint32 AdjustedHwResX = HwResX;
        quint32 AdjustedHwResY = HwResY;
        foreach(QJsonValue res, _attrs["pwg-raster-document-resolution-supported"].toObject()["value"].toArray())
        {
            QJsonObject resObj = res.toObject();
            if(resObj["units"] != 3)
            {
                continue;
            }
            quint32 tmpDiff = std::abs(int(HwResX-resObj["x"].toInt())) + std::abs(int(HwResY-resObj["y"].toInt()));
            if(tmpDiff < diff)
            {
                diff = tmpDiff;
                AdjustedHwResX = resObj["x"].toInt();
                AdjustedHwResY = resObj["y"].toInt();
            }
        }
        HwResX = AdjustedHwResX;
        HwResY = AdjustedHwResY;
    }

    if(documentFormat == Mimer::URF)
    { // Ensure symmetric resolution for URF
        HwResX = HwResY = std::min(HwResX, HwResY);

        quint32 diff = std::numeric_limits<quint32>::max();
        quint32 AdjustedHwRes = HwResX;

        QJsonArray urfSupported = _attrs["urf-supported"].toObject()["value"].toArray();
        foreach(QJsonValue us, urfSupported)
        {
            if(us.toString().startsWith("RS"))
            { //RS300[-600]
                QStringList resolutions = us.toString().mid(2).split("-");
                foreach(QString res, resolutions)
                {
                    int intRes = res.toInt();
                    quint32 tmpDiff = std::abs(int(HwResX - intRes));
                    if(tmpDiff < diff)
                    {
                        diff = tmpDiff;
                        AdjustedHwRes = intRes;
                    }
                }

                HwResX = HwResY = AdjustedHwRes;
                break;
            }
        }
    }

    QString Sides = getAttrOrDefault(jobAttrs, "sides").toString();

    if(Sides != "" && Sides != "one-sided")
    {
        if(documentFormat == Mimer::PWG)
        {
            QString DocumentSheetBack = _attrs["pwg-raster-document-sheet-back"].toObject()["value"].toString();
            if(Sides=="two-sided-long-edge")
            {
                if(DocumentSheetBack=="flipped")
                {
                    BackVFlip=true;
                }
                else if(DocumentSheetBack=="rotated")
                {
                    BackHFlip=true;
                    BackVFlip=true;
                }
            }
            else if(Sides=="two-sided-short-edge")
            {
                if(DocumentSheetBack=="flipped")
                {
                    BackHFlip=true;
                }
                else if(DocumentSheetBack=="manual-tumble")
                {
                    BackHFlip=true;
                    BackVFlip=true;
                }
            }
        }
        else if(documentFormat == Mimer::URF)
        {
            QJsonArray URfSupported = _attrs["urf-supported"].toObject()["value"].toArray();
            if(Sides=="two-sided-long-edge")
            {
                if(URfSupported.contains("DM2"))
                {
                    BackVFlip=true;
                }
                else if(URfSupported.contains("DM3"))
                {
                    BackHFlip=true;
                    BackVFlip=true;
                }
            }
            else if(Sides=="two-sided-short-edge")
            {
                if(URfSupported.contains("DM2"))
                {
                    BackHFlip=true;
                }
                else if(URfSupported.contains("DM4"))
                {
                    BackHFlip=true;
                    BackVFlip=true;
                }
            }
        }
    }

}

void IppPrinter::print(QJsonObject jobAttrs, QString filename)
{
    qDebug() << "printing" << filename << jobAttrs;

    _progress = "";
    emit progressChanged();

    QFileInfo fileinfo(filename);

    if(!fileinfo.exists())
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

    QJsonObject o = opAttrs();
    o.insert("job-name", QJsonObject {{"tag", IppMsg::NameWithoutLanguage}, {"value", fileinfo.fileName()}});

    QString PaperSize = getAttrOrDefault(jobAttrs, "media").toString();

    if(jobAttrs.contains("media-col") && jobAttrs.contains("media"))
    {
        qDebug() << "moving media to media-col" << PaperSize;
        if(!PaperSizes.contains(PaperSize))
        {
            emit convertFailed(tr("Unknown document format dimensions"));
            return;
        }

        int x = PaperSizes[PaperSize].width()*100;
        int y = PaperSizes[PaperSize].height()*100;

        QJsonObject Dimensions =
            {{"tag", IppMsg::BeginCollection},
             {"value", QJsonObject { {"x-dimension", QJsonObject{{"tag", IppMsg::Integer}, {"value", x}}},
                                     {"y-dimension", QJsonObject{{"tag", IppMsg::Integer}, {"value", y}}} }
             }};

        // TODO: make a setter function
        QJsonObject MediaCol = jobAttrs["media-col"].toObject();
        QJsonObject MediaColValue = MediaCol["value"].toObject();
        MediaColValue["media-size"] = Dimensions;
        MediaCol["value"] = MediaColValue;
        MediaCol["tag"] = IppMsg::BeginCollection;
        jobAttrs["media-col"] = MediaCol;

        jobAttrs.remove("media");
    }

    QString documentFormat = getAttrOrDefault(jobAttrs, "document-format").toString();
    qDebug() << "target format:" << documentFormat;

    // document-format goes in the op-attrs and not the job-attrs
    o.insert("document-format", QJsonObject {{"tag", IppMsg::MimeMediaType}, {"value", documentFormat}});
    jobAttrs.remove("document-format");

    documentFormat = targetFormatIfAuto(documentFormat, mimeType, supportedMimeTypes);
    qDebug() << "adjusted target format:" << documentFormat;

    if(documentFormat == "" || documentFormat == Mimer::OctetStream)
    {
        emit convertFailed(tr("Unknown document format"));
        return;
    }

    qDebug() << "Printing job" << o << jobAttrs;

    QJsonValue PrinterResolutionRef = getAttrOrDefault(jobAttrs, "printer-resolution");
    quint32 HwResX = PrinterResolutionRef.toObject()["x"].toInt(300);
    quint32 HwResY = PrinterResolutionRef.toObject()["y"].toInt(300);
    bool BackHFlip = false;
    bool BackVFlip = false;

    adjustRasterSettings(documentFormat, jobAttrs, HwResX, HwResY, BackHFlip, BackVFlip);

    quint32 Quality = getAttrOrDefault(jobAttrs, "print-quality").toInt();

    QString PrintColorMode = getAttrOrDefault(jobAttrs, "print-color-mode").toString();
    quint32 Colors = PrintColorMode.contains("color") ? 3 : PrintColorMode.contains("monochrome") ? 1 : 0;

    quint32 PageRangeLow = 0;
    quint32 PageRangeHigh = 0;
    if(jobAttrs.contains("page-ranges"))
    {
        QJsonObject PageRanges = getAttrOrDefault(jobAttrs, "page-ranges").toObject();
        PageRangeLow = PageRanges["low"].toInt();
        PageRangeHigh = PageRanges["high"].toInt();
        // Effected locally, unless it is Postscript which we cant't render
        if(documentFormat != Mimer::Postscript)
        {
            jobAttrs.remove("page-ranges");
        }
    }

    qDebug() << "Final op attributes:" << o;
    qDebug() << "Final job attributes:" << jobAttrs;

    IppMsg job = mk_msg(o, jobAttrs);
    Bytestream contents = job.encode(IppMsg::PrintJob);

    // Shouldn't and can't process these formats respectively
    if((mimeType == documentFormat) && (documentFormat == Mimer::JPEG || documentFormat == Mimer::Postscript))
    {
        emit doJustUpload(filename, contents);
    }
    else
    {
        if(PaperSize == "")
        {
            PaperSize = "iso_a4_210x297mm";
        }
        else if(!PaperSizes.contains(PaperSize))
        {
            emit convertFailed(tr("Unsupported print media"));
            return;
        }

        bool TwoSided = false;
        bool Tumble = false;
        QString Sides = getAttrOrDefault(jobAttrs, "sides").toString();

        if(Sides=="two-sided-long-edge")
        {
            TwoSided = true;
        }
        else if(Sides=="two-sided-short-edge")
        {
            TwoSided = true;
            Tumble = true;
        }

        setBusyMessage(tr("Preparing"));

        if(mimeType == Mimer::PDF)
        {
            emit doConvertPdf(filename, contents, documentFormat, Colors, Quality,
                              PaperSize, HwResX, HwResY, TwoSided, Tumble, PageRangeLow, PageRangeHigh,
                              BackHFlip, BackVFlip);
        }
        else if(mimeType == Mimer::Plaintext)
        {
            emit doConvertPlaintext(filename, contents, documentFormat, Colors, Quality,
                                    PaperSize, HwResX, HwResY, TwoSided, Tumble, BackHFlip, BackVFlip);
        }
        else if (Mimer::isImage(mimeType))
        {
            QMargins margins(getAttrOrDefault(jobAttrs, "media-left-margin", "media-col").toInt(),
                             getAttrOrDefault(jobAttrs, "media-top-margin", "media-col").toInt(),
                             getAttrOrDefault(jobAttrs, "media-right-margin", "media-col").toInt(),
                             getAttrOrDefault(jobAttrs, "media-bottom-margin", "media-col").toInt());

            emit doConvertImage(filename, contents, documentFormat, Colors, Quality,
                                PaperSize, HwResX, HwResY, margins);
        }
        else if(Mimer::isOffice(mimeType))
        {
            emit doConvertOfficeDocument(filename, contents, documentFormat, Colors, Quality,
                                         PaperSize, HwResX, HwResY, TwoSided, Tumble, PageRangeLow, PageRangeHigh,
                                         BackHFlip, BackVFlip);
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

    emit doGetJobs(job.encode(IppMsg::GetJobs));

    return true;
}

bool IppPrinter::cancelJob(qint32 jobId) {

    qDebug() << "cancelling jobs";

    QJsonObject o = opAttrs();
    o.insert("job-id", QJsonObject {{"tag", IppMsg::Integer}, {"value", jobId}});

    IppMsg job = IppMsg(o, QJsonObject());

    emit doCancelJob(job.encode(IppMsg::CancelJob));

    return true;
}

bool IppPrinter::correctSuffix()
{
    foreach(QJsonValue u, _attrs["printer-uri-supported"].toObject()["value"].toArray())
    {
        QUrl url(u.toString());
        if(url.path() == _url.path())
        {
            return true;
        }
    }
    return false;
}

QStringList IppPrinter::suffixes()
{
    QStringList res;
    foreach(QJsonValue u, _attrs["printer-uri-supported"].toObject()["value"].toArray())
    {
        QUrl url(u.toString());
        if(!res.contains(url.path()))
        {
            res.append(url.path());
        }
    }
    res.sort();
    return res;
}

QUrl IppPrinter::httpUrl() {
    qDebug() << _url;
    QUrl url = _url;
    if(url.scheme() == "ipps")
    {
        url.setScheme("https");
        if(url.port() == -1) {
            url.setPort(443);
        }
    }
    else
    {
        url.setScheme("http");
        if(url.port() == -1) {
            url.setPort(631);
        }
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

QJsonValue IppPrinter::getAttrOrDefault(QJsonObject jobAttrs, QString name, QString subkey)
{
    if(subkey == "")
    {
        if(jobAttrs.contains(name))
        {
            return jobAttrs[name].toObject()["value"];
        }
        else
        {
            return _attrs[name+"-default"].toObject()["value"];
        }
    }
    else
    {
        QJsonObject subObj = jobAttrs[subkey].toObject()["value"].toObject();
        if(subObj.contains(name))
        {
            return subObj[name].toObject()["value"];
        }
        else
        {
            return _attrs[name+"-default"].toObject()["value"];
        }
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
