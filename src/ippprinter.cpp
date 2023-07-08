#include "ippprinter.h"
#include "ippdiscovery.h"
#include "mimer.h"
#include "convertchecker.h"
#include "papersizes.h"
#include "overrider.h"
#include "settings.h"
#include <fstream>
#include <QRegularExpression>
#include <QTemporaryFile>
#include <QFileInfo>

Q_DECLARE_METATYPE(QMargins)
Q_DECLARE_METATYPE(IppMsg)

IppPrinter::IppPrinter()
{
    _worker = new PrinterWorker(this);

    QObject::connect(this, &IppPrinter::urlChanged, this, &IppPrinter::onUrlChanged);
    qRegisterMetaType<QTemporaryFile*>("QTemporaryFile*");

    connect(this, &IppPrinter::doDoGetPrinterAttributes, _worker, &PrinterWorker::getPrinterAttributes);
    connect(this, &IppPrinter::doGetJobs, _worker, &PrinterWorker::getJobs);
    connect(this, &IppPrinter::doCancelJob, _worker, &PrinterWorker::cancelJob);
    connect(this, &IppPrinter::doIdentify, _worker, &PrinterWorker::identify);
    connect(this, &IppPrinter::doPrint, _worker, &PrinterWorker::print);
    connect(this, &IppPrinter::doPrint2, _worker, &PrinterWorker::print2);

    connect(this, &IppPrinter::doGetStrings, _worker, &PrinterWorker::getStrings);
    connect(this, &IppPrinter::doGetImage, _worker, &PrinterWorker::getImage);

    connect(_worker, &PrinterWorker::progress, this, &IppPrinter::setProgress);
    connect(_worker, &PrinterWorker::busyMessage, this, &IppPrinter::setBusyMessage);
    connect(_worker, &PrinterWorker::failed, this, &IppPrinter::convertFailed);

    qRegisterMetaType<QMargins>();
    qRegisterMetaType<IppMsg>();
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
    QMetaObject::invokeMethod(_worker, "urlChanged", Q_ARG(QUrl, httpUrl()));
    refresh();
}

void IppPrinter::refresh() {

    if(_url.scheme() == "file")
    {
        _attrs = QJsonObject();
        QString localFile = _url.toLocalFile();

        if(localFile.endsWith(".raw") || localFile.endsWith(".bin"))
        {
            try
            {
                std::ifstream ifs = std::ifstream(localFile.toStdString(), std::ios::in | std::ios::binary);
                Bytestream InBts(ifs);
                IppMsg resp(InBts);
                qDebug() << resp.getStatus() << resp.getOpAttrs() << resp.getPrinterAttrs();
                _attrs = resp.getPrinterAttrs();
            }
            catch(const std::exception& e)
            {
                qDebug() << e.what();
            }
        }
        else
        {
            QFile file(localFile);
            if(file.open(QIODevice::ReadOnly))
            {
                QJsonDocument JsonDocument = QJsonDocument::fromJson(file.readAll());
                _attrs = JsonDocument.object();
                file.close();
            }
        }
        Overrider::instance()->apply(_attrs);
        emit attrsChanged();

        // For testing fake file-prinetrs with a strings file hosted elsewhere
        // MaybeGetIcon();
        // MaybeGetStrings();

        UpdateAdditionalDocumentFormats();
    }
    else
    {
        QJsonObject o = opAttrs();

        IppMsg msg = IppMsg(IppMsg::GetPrinterAttrs, o);
        emit doDoGetPrinterAttributes(msg.encode());
    }
}

void IppPrinter::MaybeGetStrings()
{
    if(_attrs.contains("printer-strings-uri") && _strings.empty())
    {
        QUrl url(_attrs["printer-strings-uri"].toObject()["value"].toString());
        resolveUrl(url);
        if(isAllowedAddress(url))
        {
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

        resolveUrl(url);

        if(isAllowedAddress(url))
        {
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
        for(QByteArray l : bl)
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
        else if(documentFormat == Mimer::SVG)
        {
            QStringList SvgPrioList {Mimer::PWG, Mimer::URF, Mimer::PDF, Mimer::Postscript};
            return firstMatch(supportedMimeTypes, SvgPrioList);
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

void IppPrinter::adjustRasterSettings(QString filename, QString mimeType, QJsonObject& jobAttrs, PrintParameters& Params)
{
    if(Params.format != PrintParameters::PWG && Params.format != PrintParameters::URF)
    {
        return;
    }

    jobAttrs.remove("printer-resolution");

    if(Params.format == PrintParameters::PWG)
    {
        quint32 diff = std::numeric_limits<quint32>::max();
        quint32 AdjustedHwResX = Params.hwResW;
        quint32 AdjustedHwResY = Params.hwResH;
        for(QJsonValue res : _attrs["pwg-raster-document-resolution-supported"].toObject()["value"].toArray())
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
    else if(Params.format == PrintParameters::URF)
    { // Ensure Params.format resolution for URF
        Params.hwResW = Params.hwResH = std::min(Params.hwResW, Params.hwResH);

        quint32 diff = std::numeric_limits<quint32>::max();
        quint32 AdjustedHwRes = Params.hwResW;

        QJsonArray urfSupported = _attrs["urf-supported"].toObject()["value"].toArray();
        for(QJsonValue us : urfSupported)
        {
            if(us.toString().startsWith("RS"))
            { //RS300[-600]
                QStringList resolutions = us.toString().mid(2).split("-");
                for(QString res : resolutions)
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

    if(Params.format == PrintParameters::PWG)
    {
        QString DocumentSheetBack = _attrs["pwg-raster-document-sheet-back"].toObject()["value"].toString();
        if(DocumentSheetBack=="flipped")
        {
            Params.backXformMode=PrintParameters::Flipped;
        }
        else if(DocumentSheetBack=="rotated")
        {
            Params.backXformMode=PrintParameters::Rotated;
        }
        else if(DocumentSheetBack=="manual-tumble")
        {
            Params.backXformMode=PrintParameters::ManualTumble;
        }
    }
    else if(Params.format == PrintParameters::URF)
    {
        QJsonArray URfSupported = _attrs["urf-supported"].toObject()["value"].toArray();
        if(URfSupported.contains("DM2"))
        {
            Params.backXformMode=PrintParameters::Flipped;
        }
        else if(URfSupported.contains("DM3"))
        {
            Params.backXformMode=PrintParameters::Rotated;
        }
        else if(URfSupported.contains("DM4"))
        {
            Params.backXformMode=PrintParameters::ManualTumble;
        }
    }

    int copies_requested = getAttrOrDefault(jobAttrs, "copies").toInt(1);
    QJsonArray varying_attributes = _attrs["document-format-varying-attributes"].toObject()["value"].toArray();
    bool supports_copies = _attrs.contains("copies-supported")
                         && _attrs["copies-supported"].toObject()["value"].toObject()["high"].toInt(1) != 1
                         // assume raster formats causes the variation in supported copies
                         && !(varying_attributes.contains("copies-supported") || varying_attributes.contains("copies"));

    if(copies_requested > 1 && !supports_copies)
    {
        QString copyMode = getAttrOrDefault(jobAttrs, "multiple-document-handling").toString();
        qDebug() << "Doing local copies" << copyMode << copies_requested;
        Params.copies = copies_requested;
        if(copyMode == "separate-documents-uncollated-copies")
        { // Only do silly copies if explicitly requested
            Params.collatedCopies = false;
        }
        jobAttrs.remove("copies");


        QString Sides = getAttrOrDefault(jobAttrs, "sides").toString();

        if(Sides != "one-sided")
        {
            bool singlePageRange = false;
            if(Params.pageRangeList.size() == 1)
            {
                size_t fromPage = Params.pageRangeList.begin()->first;
                size_t toPage = Params.pageRangeList.begin()->second;
                singlePageRange = fromPage != 0 && fromPage == toPage;
            }
            bool singlePageDocument = (Mimer::instance()->isImage(mimeType) ||
                                      (mimeType == Mimer::PDF && ConvertChecker::instance()->pdfPages(filename) == 1));

            if(singlePageDocument || singlePageRange)
            {
                jobAttrs.insert("sides", QJsonObject {{"tag", IppMsg::Keyword}, {"value", "one-sided"}});
                qDebug() << "Optimizing one-page document to one-sided";
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

    if(!Params.setPaperSize(getAttrOrDefault(jobAttrs, "media").toString(Params.paperSizeName.c_str()).toStdString()))
    {
        emit convertFailed(tr("Unsupported paper size"));
        return;
    }

    QString targetFormat = getAttrOrDefault(jobAttrs, "document-format").toString();
    QStringList goodFormats = {Mimer::PDF, Mimer::Postscript, Mimer::PWG, Mimer::URF};
    bool imageToImage = Mimer::isImage(mimeType) && Mimer::isImage(targetFormat);
    if(!jobAttrs.contains("document-format") && !(goodFormats.contains(targetFormat) || imageToImage))
    { // User made no choice, and we don't know the target format - treat as if auto
        targetFormat = Mimer::OctetStream;
    }


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

        int x = Params.getPaperSizeWInMillimeters()*100;
        int y = Params.getPaperSizeHInMillimeters()*100;

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

    QJsonObject jobOpAttrs = opAttrs();
    // document-format goes in the op-attrs and not the job-attrs
    jobOpAttrs.insert("document-format", QJsonObject {{"tag", IppMsg::MimeMediaType}, {"value", targetFormat}});
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
    else
    {
        Params.format = PrintParameters::Invalid;
    }

    qDebug() << "Printing job" << jobOpAttrs << jobAttrs;

    QJsonValue PrinterResolutionRef = getAttrOrDefault(jobAttrs, "printer-resolution");
    Params.hwResW = PrinterResolutionRef.toObject()["x"].toInt(Params.hwResW);
    Params.hwResH = PrinterResolutionRef.toObject()["y"].toInt(Params.hwResH);

    // Effected locally, unless it is Postscript which we cant't render
    if(jobAttrs.contains("page-ranges") && mimeType != Mimer::Postscript)
    {
        QJsonArray tmp;
        QJsonValue pageRanges = getAttrOrDefault(jobAttrs, "page-ranges");
        if(pageRanges.isArray())
        {
            tmp = pageRanges.toArray();
        }
        else if(pageRanges.isObject())
        {
            tmp = {pageRanges.toObject()};
        }
        for(QJsonValueRef ref : tmp)
        {
            Params.pageRangeList.push_back({ref.toObject()["low"].toInt(), ref.toObject()["high"].toInt()});
        }
        jobAttrs.remove("page-ranges");
    }

    adjustRasterSettings(filename, mimeType, jobAttrs, Params);

    QString Sides = getAttrOrDefault(jobAttrs, "sides").toString();

    if(Sides=="two-sided-long-edge")
    {
        Params.duplexMode = PrintParameters::TwoSidedLongEdge;
    }
    else if(Sides=="two-sided-short-edge")
    {
        Params.duplexMode = PrintParameters::TwoSidedShortEdge;
    }

    switch (getAttrOrDefault(jobAttrs, "print-quality").toInt())
    {
    case 3:
        Params.quality = PrintParameters::DraftQuality;
        break;
    case 4:
        Params.quality = PrintParameters::NormalQuality;
        break;
    case 5:
        Params.quality = PrintParameters::HighQuality;
        break;
    default:
        Params.quality = PrintParameters::DefaultQuality;
        break;
    }

    bool supportsColor = _attrs["operations-supported"].toObject()["value"].toArray().contains("color");
    QString PrintColorMode = getAttrOrDefault(jobAttrs, "print-color-mode").toString();
    Params.colorMode = PrintColorMode.contains("color") ? PrintParameters::sRGB24
                     : PrintColorMode.contains("monochrome") || !supportsColor ? PrintParameters::Gray8
                     : Params.colorMode;

    qDebug() << "Color mode" << Params.colorMode;

    QJsonArray supportedOperations = _attrs["operations-supported"].toObject()["value"].toArray();

    if(supportedOperations.contains(IppMsg::CreateJob) && supportedOperations.contains(IppMsg::SendDocument))
    {
        QJsonObject createJobOpAttrs = opAttrs();
        createJobOpAttrs.insert("job-name", QJsonObject {{"tag", IppMsg::NameWithoutLanguage}, {"value", fileinfo.fileName()}});

        qDebug() << "Final create op attributes:" << createJobOpAttrs;
        qDebug() << "Final job attributes:" << jobAttrs;

        IppMsg createJob = mk_msg(IppMsg::CreateJob, createJobOpAttrs, jobAttrs);
        qDebug() << "Final job op attributes:" << jobOpAttrs;

        IppMsg sendDoc = mk_msg(IppMsg::SendDocument, jobOpAttrs);

        emit doPrint2(filename, mimeType, targetFormat, createJob, sendDoc, Params, margins);
    }
    else
    {
        jobOpAttrs.insert("job-name", QJsonObject {{"tag", IppMsg::NameWithoutLanguage}, {"value", fileinfo.fileName()}});

        qDebug() << "Final op attributes:" << jobOpAttrs;
        qDebug() << "Final job attributes:" << jobAttrs;

        IppMsg job = mk_msg(IppMsg::PrintJob, jobOpAttrs, jobAttrs);
        emit doPrint(filename, mimeType, targetFormat, job, Params, margins);
    }

}

bool IppPrinter::getJobs() {

    qDebug() << "getting jobs";
    QJsonObject o = opAttrs();
    o.insert("requested-attributes", QJsonObject {{"tag", IppMsg::Keyword}, {"value", "all"}});

    IppMsg job = IppMsg(IppMsg::GetJobs, o, QJsonObject());

    emit doGetJobs(job.encode());

    return true;
}

bool IppPrinter::cancelJob(qint32 jobId) {

    qDebug() << "cancelling jobs";

    QJsonObject o = opAttrs();
    o.insert("job-id", QJsonObject {{"tag", IppMsg::Integer}, {"value", jobId}});

    IppMsg job = IppMsg(IppMsg::CancelJob, o, QJsonObject());

    emit doCancelJob(job.encode());

    return true;
}

bool IppPrinter::identify() {

    qDebug() << "identifying";

    QJsonObject o = opAttrs();

    IppMsg job = IppMsg(IppMsg::IdentifyPrinter, o, QJsonObject());

    emit doCancelJob(job.encode());

    return true;
}

bool IppPrinter::correctSuffix()
{
    for(QJsonValue u : _attrs["printer-uri-supported"].toObject()["value"].toArray())
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
    for(QJsonValue u : _attrs["printer-uri-supported"].toObject()["value"].toArray())
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

IppMsg IppPrinter::mk_msg(IppMsg::Operation operation, QJsonObject opAttrs, QJsonObject jobAttrs)
{
    if(_attrs.contains("ipp-versions-supported") &&
       _attrs["ipp-versions-supported"].toObject()["value"].toArray().contains("2.0"))
    {
        qDebug() << "TWO-POINT-ZERO";
        return IppMsg(operation, opAttrs, jobAttrs, 2, 0);
    }
    return IppMsg(operation, opAttrs, jobAttrs);
}

void IppPrinter::resolveUrl(QUrl& url)
{
    if(!IppDiscovery::instance()->resolve(url))
    { // If "proper" resolution fails, cheat...
        QString host = url.host();

        if(host.endsWith("."))
        {
            host.chop(1);
        }

        QString dnsSdName = _attrs["printer-dns-sd-name"].toObject()["value"].toString();
        dnsSdName = dnsSdName.append(".local");

        if(host.compare(dnsSdName, Qt::CaseInsensitive) == 0)
        { // This could be done unconditionally, but some might want their externally hosted stuff to work
            url.setHost(_url.host());
        }
    }
}
