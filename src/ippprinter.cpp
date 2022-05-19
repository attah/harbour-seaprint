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
    connect(this, &IppPrinter::doIdentify, &_worker, &PrinterWorker::identify);
    connect(this, &IppPrinter::doJustUpload, &_worker, &PrinterWorker::justUpload);
    connect(this, &IppPrinter::doFixupPlaintext, &_worker, &PrinterWorker::fixupPlaintext);
    connect(this, &IppPrinter::doFixupImage, &_worker, &PrinterWorker::fixupImage);

    connect(this, &IppPrinter::doConvertPdf, &_worker, &PrinterWorker::convertPdf);
    connect(this, &IppPrinter::doConvertImage, &_worker, &PrinterWorker::convertImage);
    connect(this, &IppPrinter::doConvertOfficeDocument, &_worker, &PrinterWorker::convertOfficeDocument);
    connect(this, &IppPrinter::doConvertPlaintext, &_worker, &PrinterWorker::convertPlaintext);

    connect(this, &IppPrinter::doGetStrings, &_worker, &PrinterWorker::getStrings);
    connect(this, &IppPrinter::doGetImage, &_worker, &PrinterWorker::getImage);

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

//        MaybeGetStrings(); - for testing fake file-prinetrs with a strings file hosted elsewhere
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
        if(isAllowedAddress(url))
        {
            IppDiscovery::instance()->resolve(url);
            emit doGetStrings(url);
        }
    }
}

void IppPrinter::MaybeGetIcon(bool retry)
{
    if(_attrs.contains("printer-icons") && (_icon.isNull() || retry) && !_iconRetried)
    {
        QUrl url;
        QJsonArray icons = _attrs["printer-icons"].toObject()["value"].toArray();

        if(retry)
        { // If there were more than one icon, try the last one on the retry
            if(icons.size() > 1)
            {
                url = icons.last().toString();
            }
        }
        else
        {
            if(icons.size() == 3)
            { // If there are 3 icons, the first will be the 48px one, ignore it
                url = icons.at(1).toString();
            }
            else
            {
                url = icons.at(0).toString();
            }
        }

        if(isAllowedAddress(url))
        {
            IppDiscovery::instance()->resolve(url);
            emit doGetImage(url);
        }
    }
    _iconRetried = retry;
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
    MaybeGetIcon();
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

void IppPrinter::identifyFinished(CURLcode /*res*/, Bytestream /*data*/)
{

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

void IppPrinter::getImageFinished(CURLcode res, Bytestream data)
{
    qDebug() << res << data.size();
    if(res == CURLE_OK)
    {
        QImage tmp;
        if(tmp.loadFromData(data.raw(), data.size(), "PNG"))
        {
            _icon = tmp;
            qDebug() << "image loaded" << _icon;
            emit iconChanged();

            if(tmp.size().width() < 128)
            {
                MaybeGetIcon(true);
            }
        }
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
        QStringList PdfPrioList = {Mimer::PDF, Mimer::Postscript, Mimer::PWG, Mimer::URF};
        if(mimeType == Mimer::PDF)
        {
            return firstMatch(supportedMimeTypes, PdfPrioList);
        }
        else if(mimeType == Mimer::Postscript)
        {
            return firstMatch(supportedMimeTypes, {Mimer::Postscript});
        }
        else if(mimeType == Mimer::Plaintext)
        {
            return firstMatch(supportedMimeTypes, PdfPrioList << Mimer::Plaintext);
        }
        else if(Mimer::isOffice(mimeType))
        {
            return firstMatch(supportedMimeTypes, PdfPrioList);
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

void IppPrinter::adjustRasterSettings(QString documentFormat, QJsonObject& jobAttrs, PrintParameters& Params)
{
    if(documentFormat != Mimer::PWG && documentFormat != Mimer::URF)
    {
        return;
    }

    //TODO? jobAttrs.remove("printer-resolution");

    if(documentFormat == Mimer::PWG)
    {
        quint32 diff = std::numeric_limits<quint32>::max();
        quint32 AdjustedHwResX = Params.hwResW;
        quint32 AdjustedHwResY = Params.hwResH;
        foreach(QJsonValue res, _attrs["pwg-raster-document-resolution-supported"].toObject()["value"].toArray())
        {
            QJsonObject resObj = res.toObject();
            if(resObj["units"] != 3)
            {
                continue;
            }
            quint32 tmpDiff = std::abs(int(Params.hwResW-resObj["x"].toInt())) + std::abs(int(Params.hwResH-resObj["y"].toInt()));
            if(tmpDiff < diff)
            {
                diff = tmpDiff;
                AdjustedHwResX = resObj["x"].toInt();
                AdjustedHwResY = resObj["y"].toInt();
            }
        }
        Params.hwResW = AdjustedHwResX;
        Params.hwResH = AdjustedHwResY;
    }

    if(documentFormat == Mimer::URF)
    { // Ensure symmetric resolution for URF
        Params.hwResW = Params.hwResH = std::min(Params.hwResW, Params.hwResH);

        quint32 diff = std::numeric_limits<quint32>::max();
        quint32 AdjustedHwRes = Params.hwResW;

        QJsonArray urfSupported = _attrs["urf-supported"].toObject()["value"].toArray();
        foreach(QJsonValue us, urfSupported)
        {
            if(us.toString().startsWith("RS"))
            { //RS300[-600]
                QStringList resolutions = us.toString().mid(2).split("-");
                foreach(QString res, resolutions)
                {
                    int intRes = res.toInt();
                    quint32 tmpDiff = std::abs(int(Params.hwResW - intRes));
                    if(tmpDiff < diff)
                    {
                        diff = tmpDiff;
                        AdjustedHwRes = intRes;
                    }
                }

                Params.hwResW = Params.hwResH = AdjustedHwRes;
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
                    Params.backVFlip=true;
                }
                else if(DocumentSheetBack=="rotated")
                {
                    Params.backHFlip=true;
                    Params.backVFlip=true;
                }
            }
            else if(Sides=="two-sided-short-edge")
            {
                if(DocumentSheetBack=="flipped")
                {
                    Params.backHFlip=true;
                }
                else if(DocumentSheetBack=="manual-tumble")
                {
                    Params.backHFlip=true;
                    Params.backVFlip=true;
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
                    Params.backVFlip=true;
                }
                else if(URfSupported.contains("DM3"))
                {
                    Params.backHFlip=true;
                    Params.backVFlip=true;
                }
            }
            else if(Sides=="two-sided-short-edge")
            {
                if(URfSupported.contains("DM2"))
                {
                    Params.backHFlip=true;
                }
                else if(URfSupported.contains("DM4"))
                {
                    Params.backHFlip=true;
                    Params.backVFlip=true;
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

    PrintParameters Params;
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

    Params.paperSizeName = getAttrOrDefault(jobAttrs, "media").toString(Params.paperSizeName.c_str()).toStdString();

    QString targetFormat = getAttrOrDefault(jobAttrs, "document-format").toString();
    qDebug() << "target format:" << targetFormat;

    QMargins margins(getAttrOrDefault(jobAttrs, "media-left-margin", "media-col").toInt(),
                     getAttrOrDefault(jobAttrs, "media-top-margin", "media-col").toInt(),
                     getAttrOrDefault(jobAttrs, "media-right-margin", "media-col").toInt(),
                     getAttrOrDefault(jobAttrs, "media-bottom-margin", "media-col").toInt());

    // Only keep margin setting for JPEG - but only attemt to remove it if media-col exists
    if(!mimer->isImage(targetFormat) && jobAttrs.contains("media-col"))
    {
        QJsonObject MediaCol = jobAttrs["media-col"].toObject();
        QJsonObject MediaColValue = MediaCol["value"].toObject();

        MediaColValue.remove("media-left-margin");
        MediaColValue.remove("media-top-margin");
        MediaColValue.remove("media-right-margin");
        MediaColValue.remove("media-bottom-margin");

        if(!MediaColValue.empty())
        {
            MediaCol["value"] = MediaColValue;
            jobAttrs["media-col"] = MediaCol;
        }
        else
        {
            jobAttrs.remove("media-col");
        }
    }

    if(jobAttrs.contains("media-col") && jobAttrs.contains("media"))
    {
        qDebug() << "moving media to media-col" << Params.paperSizeName.c_str();
        if(!PaperSizes.contains(Params.paperSizeName.c_str()))
        {
            emit convertFailed(tr("Unsupported paper size"));
            return;
        }

        int x = PaperSizes[Params.paperSizeName.c_str()].width()*100;
        int y = PaperSizes[Params.paperSizeName.c_str()].height()*100;

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

    // document-format goes in the op-attrs and not the job-attrs
    o.insert("document-format", QJsonObject {{"tag", IppMsg::MimeMediaType}, {"value", targetFormat}});
    jobAttrs.remove("document-format");

    targetFormat = targetFormatIfAuto(targetFormat, mimeType, supportedMimeTypes);
    qDebug() << "adjusted target format:" << targetFormat;

    if(targetFormat == "" || targetFormat == Mimer::OctetStream)
    {
        emit convertFailed(tr("Unknown document format"));
        return;
    }

    if(targetFormat == Mimer::PDF)
    {
        Params.format = PrintParameters::PDF;
    }
    else if(targetFormat == Mimer::Postscript)
    {
        Params.format = PrintParameters::Postscript;
    }
    else if(targetFormat == Mimer::PWG)
    {
        Params.format = PrintParameters::PWG;
    }
    else if(targetFormat == Mimer::URF)
    {
        Params.format = PrintParameters::URF;
    }

    QSizeF size = PaperSizes[Params.paperSizeName.c_str()];
    Params.paperSizeUnits = PrintParameters::Millimeters;
    Params.paperSizeW = size.width();
    Params.paperSizeH = size.height();

    qDebug() << "Printing job" << o << jobAttrs;

    QJsonValue PrinterResolutionRef = getAttrOrDefault(jobAttrs, "printer-resolution");
    Params.hwResW = PrinterResolutionRef.toObject()["x"].toInt(Params.hwResW);
    Params.hwResH = PrinterResolutionRef.toObject()["y"].toInt(Params.hwResH);

    adjustRasterSettings(targetFormat, jobAttrs, Params);

    Params.quality = getAttrOrDefault(jobAttrs, "print-quality").toInt();

    QString PrintColorMode = getAttrOrDefault(jobAttrs, "print-color-mode").toString();
    Params.colors = PrintColorMode.contains("color") ? 3 : PrintColorMode.contains("monochrome") ? 1 : Params.colors;

    if(jobAttrs.contains("page-ranges"))
    {
        QJsonObject PageRanges = getAttrOrDefault(jobAttrs, "page-ranges").toObject();
        Params.fromPage = PageRanges["low"].toInt();
        Params.toPage = PageRanges["high"].toInt();
        // Effected locally, unless it is Postscript which we cant't render
        if(targetFormat != Mimer::Postscript)
        {
            jobAttrs.remove("page-ranges");
        }
    }

    qDebug() << "Final op attributes:" << o;
    qDebug() << "Final job attributes:" << jobAttrs;

    IppMsg job = mk_msg(o, jobAttrs);
    Bytestream contents = job.encode(IppMsg::PrintJob);

    setBusyMessage(tr("Preparing"));

    if((mimeType == targetFormat) && (targetFormat == Mimer::Postscript))
    { // Can't process Postscript
        emit doJustUpload(filename, contents);
    }
    else if(mimer->isImage(targetFormat))
    { // Just make sure the image is in the desired format (and jpeg baseline-encoded), don't resize locally
        emit doFixupImage(filename, contents, targetFormat);
    }
    else
    {
        QString Sides = getAttrOrDefault(jobAttrs, "sides").toString();

        if(Sides=="two-sided-long-edge")
        {
            Params.duplex = true;
        }
        else if(Sides=="two-sided-short-edge")
        {
            Params.duplex = true;
            Params.tumble = true;
        }

        if(mimeType == Mimer::PDF)
        {
            emit doConvertPdf(filename, contents, Params);
        }
        else if(mimeType == Mimer::Plaintext)
        {
            if(targetFormat == Mimer::Plaintext)
            {
                emit doFixupPlaintext(filename, contents);
            }
            else
            {
                emit doConvertPlaintext(filename, contents, Params);
            }
        }
        else if (Mimer::isImage(mimeType))
        {
            emit doConvertImage(filename, contents, Params, margins);
        }
        else if(Mimer::isOffice(mimeType))
        {
            emit doConvertOfficeDocument(filename, contents, Params);
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

bool IppPrinter::identify() {

    qDebug() << "identifying";

    QJsonObject o = opAttrs();

    IppMsg job = IppMsg(o, QJsonObject());

    emit doCancelJob(job.encode(IppMsg::IdentifyPrinter));

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

bool IppPrinter::isAllowedAddress(QUrl url)
{
    bool allowed = ((url.host() == _url.host()) || Settings::instance()->allowExternalConnections());
    qDebug() << url << " is allowed: " << allowed;
    return allowed;
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
