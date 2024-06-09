#include "qippprinter.h"
#include "qippdiscovery.h"
#include "mimer.h"
#include "convertchecker.h"
#include "papersizes.h"
#include "settings.h"
#include <fstream>
#include <QRegularExpression>
#include <QTemporaryFile>
#include <QFileInfo>
#include <QJsonDocument>


QIppPrinter::QIppPrinter()
: _job(_worker._printer.createJob())
, _sides(&_job.sides)
, _media(&_job.media)
, _copies(&_job.copies)
, _multipleDocumentHandling(&_job.multipleDocumentHandling)
, _pageRanges(&_job.pageRanges)
, _numberUp(&_job.numberUp)
, _colorMode(&_job.colorMode)
, _printQuality(&_job.printQuality)
, _resolution(&_job.resolution)
, _scaling(&_job.scaling)
, _documentFormat(&_job.documentFormat)
, _mediaType(&_job.mediaType)
, _mediaSource(&_job.mediaSource)
, _outputBin(&_job.outputBin)
, _topMargin(&_job.topMargin)
, _bottomMargin(&_job.bottomMargin)
, _leftMargin(&_job.leftMargin)
, _rightMargin(&_job.rightMargin)
{
    QObject::connect(this, &QIppPrinter::urlChanged, this, &QIppPrinter::onUrlChanged);
    QObject::connect(&_worker, &Worker::printerChanged, this, &QIppPrinter::printerChanged);
    QObject::connect(&_worker, &Worker::progressChanged, this, &QIppPrinter::onProgressChanged);
    QObject::connect(&_worker, &Worker::jobFinished, this, &QIppPrinter::onJobFinished);
    QObject::connect(&_worker, &Worker::cancelJobFailed, this, &QIppPrinter::onCancelJobFailed);
    QObject::connect(&_worker, &Worker::jobsFetched, this, &QIppPrinter::onJobsFetched);
    QObject::connect(&_worker, &Worker::iconFetched, this, &QIppPrinter::onIconFetched);
    QObject::connect(&_worker, &Worker::stringsFetched, this, &QIppPrinter::onStringsFetched);
}


QIppPrinter::~QIppPrinter() {

}

void QIppPrinter::setUrl(QString url_s)
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

    qDebug() << "Setting URL" << url_s << url;

    if(url != _url)
    {
        _url = url;
        emit urlChanged();
    }
}

void QIppPrinter::onUrlChanged()
{
    qDebug() << "onUrlChanged" << _url;

    QMetaObject::invokeMethod(&_worker, "urlChanged", Q_ARG(QUrl, _url));
    refresh();
}


void QIppPrinter::printerChanged()
{
    // TODO: only once
    _job = _worker._printer.createJob();
    emit dataChanged();
}

void QIppPrinter::onProgressChanged(size_t sent, size_t total)
{
    qDebug() << "onProgressChanged" << sent << total;
    if(total == 0)
        return;

    _progress = QString::number(100*sent/total);
    _progress += "%";
    emit progressChanged();
}

void QIppPrinter::onJobFinished(QString errMsg)
{
    emit jobFinished(errMsg);
}

void QIppPrinter::onCancelJobFailed()
{
    emit cancelJobFailed();
}


void QIppPrinter::refresh()
{
    QMetaObject::invokeMethod(&_worker, "refresh");
}

void QIppPrinter::print(QString fileName)
{
    _progress = "";
    emit progressChanged();
    setBusyMessage(tr("Printing"));
    QMetaObject::invokeMethod(&_worker, "print", Q_ARG(QString, fileName), Q_ARG(IppPrintJob, _job));
    _job = _worker._printer.createJob();
    emit dataChanged();
}

void QIppPrinter::identify()
{
    QMetaObject::invokeMethod(&_worker, "identify");
}

void QIppPrinter::getJobs()
{
    QMetaObject::invokeMethod(&_worker, "getJobs");
}

void QIppPrinter::cancelJob(int jobId)
{
    QMetaObject::invokeMethod(&_worker, "cancelJob", Q_ARG(qint32, jobId));
}

void QIppPrinter::onJobsFetched(QJsonArray jobs)
{
    _jobs = jobs;
    emit jobsChanged();
}

void QIppPrinter::onIconFetched(QImage icon)
{
    _icon = icon;
    emit iconChanged();
}

void QIppPrinter::onStringsFetched(QJsonObject strings)
{
    _strings = strings;
    emit stringsChanged();
}


QStringList QIppPrinter::getSupportedFormats()
{
    QStringList tmp;
    for(const std::string& f : _worker._printer.possibleInputFormats())
    {
        tmp.append(f.c_str());
    }
    return tmp;
}

QStringList QIppPrinter::possibleTransferFormats(QString inputFormat)
{
    QStringList tmp;
    for(const std::string& f : _worker._printer.possibleTransferFormats(inputFormat.toStdString()))
    {
        tmp.append(f.c_str());
    }
    return tmp;
}

bool QIppPrinter::getSupportsPrinterRaster()
{
    return _worker._printer.supportsPrinterRaster();
}

QString QIppPrinter::getMakeAndModel()
{
    return _worker._printer.makeAndModel().c_str();
}

QString QIppPrinter::getLocation()
{
    return _worker._printer.location().c_str();
}

int QIppPrinter::getState()
{
    return _worker._printer.state();
}

QStringList toQStringList(List<std::string> l)
{
    QStringList res;
    for(const std::string& s : l)
    {
        res.push_back(s.c_str());
    }
    return res;
}

QStringList QIppPrinter::getStateReasons()
{
    return toQStringList(_worker._printer.stateReasons());
}

QString QIppPrinter::getStateMessage()
{
    return _worker._printer.stateMessage().c_str();
}

QStringList QIppPrinter::getIppVersions()
{
    return toQStringList(_worker._printer.ippVersionsSupported());
}

QStringList QIppPrinter::getIppFeatures()
{
    return toQStringList(_worker._printer.ippFeaturesSupported());
}

int QIppPrinter::getPagesPerMinute()
{
    return _worker._printer.pagesPerMinute();
}

int QIppPrinter::getPagesPerMinuteColor()
{
    return _worker._printer.pagesPerMinuteColor();
}

QJsonArray QIppPrinter::getFirmware()
{
    QJsonArray firmwares;
    for(const IppPrinter::Firmware& firmware : _worker._printer.firmware())
    {
        firmwares.push_back(QJsonObject {{"name", firmware.name.c_str()},
                                         {"version", firmware.version.c_str()}});
    }
    return firmwares;
}

QJsonArray QIppPrinter::getSupplies()
{
    QJsonArray supplies;
    for(const IppPrinter::Supply& supply : _worker._printer.supplies())
    {
        supplies.push_back(QJsonObject {{"name", supply.name.c_str()},
                                        {"type", supply.type.c_str()},
                                        {"colors", QJsonArray::fromStringList(toQStringList(supply.colors))},
                                        {"percentage", supply.getPercent()},
                                        {"isLow", supply.isLow()}});
    }
    return supplies;
}

bool QIppPrinter::getIdentifySupported()
{
    return _worker._printer.identifySupported();
}

bool QIppPrinter::getIsWarningState()
{
    return _worker._printer.isWarningState();
}

QJsonObject QIppPrinter::getDebugJSON()
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson(_worker._printer.attributes().toJSON().dump().c_str());
    return jsonDocument.object();
}

bool QIppPrinter::getIsOk()
{
    return !_worker._printer.attributes().empty();
}

bool QIppPrinter::correctSuffix()
{
    return suffixes().contains(_url.path());
}

QStringList QIppPrinter::suffixes()
{
    QStringList res;
    for(std::string s : _worker._printer.urisSupported())
    {
        QUrl url(QString::fromStdString(s));
        if(url.isValid())
        {
            res.push_back(url.path());
        }
    }
    res.removeDuplicates();
    return res;
}

void QIppPrinter::setBusyMessage(QString msg)
{
    _busyMessage = msg;
    emit busyMessageChanged();
}

bool QIppPrinter::isAllowedAddress(QUrl url)
{
    bool allowed = ((url.host() == _url.host()) || Settings::instance()->allowExternalConnections());
    qDebug() << url << " is allowed: " << allowed;
    return allowed;
}

Worker::Worker()
{
    _thread.reset(new QThread);
    moveToThread(_thread.get());
    _thread->start();
}

Worker::~Worker()
{
    QMetaObject::invokeMethod(this, "cleanup");
    _thread->wait();
}

void Worker::cleanup()
{
    _thread->quit();
}


void Worker::urlChanged(QUrl url)
{
    qDebug() << "initing printer" << url;
    SslConfig sslConfig(!(Settings::instance()->ignoreSslErrors()));
    _printer = IppPrinter(url.toString().toStdString(), sslConfig);
    qDebug() << _printer.attributes().toJSON().dump().c_str();
    emit printerChanged();


    for(const std::string& iconUrl : _printer.icons())
    {
        QUrl iconQUrl(iconUrl.c_str());
        iconQUrl.setHost(url.host()); // Assume hosted on printer to not need to faff with name resolution
        CurlHttpGetter cr(iconQUrl.toString().toStdString(), sslConfig);
        Bytestream resMsg;
        CURLcode res = cr.await(&resMsg);
        if(res == CURLE_OK)
        {
            QImage icon;
            if(icon.loadFromData(resMsg.raw(), resMsg.size(), "PNG"))
            {
                emit iconFetched(icon);
                if(icon.size().width() >= 128)
                {
                    break;
                }
            }
        }
    }
    if(_printer.strings() != "")
    {
        QUrl stringsUrl(_printer.strings().c_str());
        stringsUrl.setHost(url.host()); // Assume hosted on printer to not need to faff with name resolution
        CurlHttpGetter cr(stringsUrl.toString().toStdString(), sslConfig);
        Bytestream resMsg;
        CURLcode res = cr.await(&resMsg);
        if(res == CURLE_OK)
        {
            QJsonObject strings;
            QByteArray ba((char*)resMsg.raw(), resMsg.size());
            // "media-type.com.epson-coated" = "Epson Photo Quality Ink Jet";
            QRegularExpression re("^\\\"(.*)\\\"\\s*=\\s*\\\"(.*)\\\";");
            QList<QByteArray> bl = ba.split('\n');
            for(QByteArray l : bl)
            {
                QRegularExpressionMatch match = re.match(l);
                if(match.hasMatch())
                {
                    strings[match.captured(1)] = match.captured(2);
                }
            }
            emit stringsFetched(strings);
        }
    }
}

void Worker::refresh()
{
    qDebug() << "resfresh";
    SslConfig sslConfig(!(Settings::instance()->ignoreSslErrors()));
    _printer.setSslconfig(sslConfig);
    _printer.refresh();
    emit printerChanged();
}

void Worker::print(QString fileName, IppPrintJob job)
{
    QString mimeType = Mimer::instance()->get_type(fileName);
    int pages = 0;
    if(mimeType == Mimer::PDF)
    {
        pages = ConvertChecker::instance()->pdfPages(fileName);
    }
    ProgressFun progressFun([this](size_t done, size_t total) -> void
                {
                    qDebug() << "fun" << done << total;
                    emit progressChanged(done, total);
                });
    Error err = _printer.runJob(job, fileName.toStdString(), mimeType.toStdString(), pages, progressFun);
    emit jobFinished(err.value_or("").c_str());
}

void Worker::identify()
{
    _printer.identify();
}

void Worker::getJobs()
{
    List<IppPrinter::JobInfo> jobInfos;
    Error err = _printer.getJobs(jobInfos);
    if(err)
    {
        qDebug() << "getJobs failed: " << err.value().c_str();
    }
    QJsonArray jobs;
    for(const IppPrinter::JobInfo& jobInfo : jobInfos)
    {
        jobs.push_back(QJsonObject {{"id", jobInfo.id},
                                    {"name", jobInfo.name.c_str()},
                                    {"state", jobInfo.state},
                                    {"stateMessage", jobInfo.stateMessage.c_str()}});
    }
    emit jobsFetched(jobs);
}

void Worker::cancelJob(int jobId)
{
    Error err = _printer.cancelJob(jobId);
    if(err)
    {
        qDebug() << "cancelJob failed: " << err.value().c_str();
        emit cancelJobFailed();
    }
    getJobs();
}
