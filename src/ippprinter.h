#ifndef IPPPRINTER_H
#define IPPPRINTER_H

#include "ippmsg.h"
#include "printerworker.h"
#include "curlrequester.h"
#include <QImage>


class IppPrinter : public QObject
{
    Q_OBJECT

    friend class PrinterWorker;

    Q_PROPERTY(QString url READ getUrl WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QJsonObject attrs MEMBER _attrs NOTIFY attrsChanged)
    Q_PROPERTY(QJsonObject jobAttrs MEMBER _jobAttrs NOTIFY jobAttrsChanged)
    Q_PROPERTY(QJsonArray jobs MEMBER _jobs NOTIFY jobsChanged)
    Q_PROPERTY(QJsonObject strings MEMBER _strings NOTIFY stringsChanged)
    Q_PROPERTY(QImage icon MEMBER _icon NOTIFY iconChanged)
    Q_PROPERTY(QStringList additionalDocumentFormats MEMBER _additionalDocumentFormats NOTIFY additionalDocumentFormatsChanged)
    Q_PROPERTY(QString busyMessage MEMBER _busyMessage NOTIFY busyMessageChanged)
    Q_PROPERTY(QString progress MEMBER _progress NOTIFY progressChanged)

    Q_PROPERTY(bool correctSuffix READ correctSuffix NOTIFY attrsChanged)
    Q_PROPERTY(QStringList suffixes READ suffixes NOTIFY attrsChanged)

public:
    IppPrinter();
    ~IppPrinter();


    QString getUrl() {return _url.toString();}
    void setUrl(QString url);

    Q_INVOKABLE void refresh();

    Q_INVOKABLE bool getJobs();
    Q_INVOKABLE bool cancelJob(qint32 jobId);
    Q_INVOKABLE bool identify();

    bool correctSuffix();
    QStringList suffixes();

signals:
    void urlChanged();
    void attrsChanged();
    void jobAttrsChanged();
    void jobsChanged();

    void stringsChanged();
    void iconChanged();

    void jobFinished(bool status);
    void cancelStatus(bool status);

    void doDoGetPrinterAttributes(Bytestream msg);
    void doGetJobs(Bytestream msg);
    void doCancelJob(Bytestream msg);
    void doIdentify(Bytestream msg);

    void doJustUpload(QString filename, Bytestream header);
    void doFixupPlaintext(QString filename, Bytestream header);
    void doPrintImageAsImage(QString filename, Bytestream header, QString targetFormat);

    void doConvertPdf(QString filename, Bytestream header, PrintParameters Params);

    void doConvertImage(QString filename,  Bytestream header, PrintParameters Params, QMargins margins);

    void doConvertOfficeDocument(QString filename, Bytestream header, PrintParameters Params);

    void doConvertPlaintext(QString filename, Bytestream header, PrintParameters Params);

    void doPrint(QString filename, QString mimeType, QString targetFormat, IppMsg job, PrintParameters Params, QMargins margins);

    void doGetStrings(QUrl url);
    void doGetImage(QUrl url);

    void additionalDocumentFormatsChanged();
    void busyMessageChanged();
    void progressChanged();

public slots:
    void print(QJsonObject attrs, QString file);


    void onUrlChanged();
    void MaybeGetStrings();
    void MaybeGetIcon(bool retry=false);
    void UpdateAdditionalDocumentFormats();
    void getPrinterAttributesFinished(CURLcode res, Bytestream data);
    void printRequestFinished(CURLcode res, Bytestream data);
    void getJobsRequestFinished(CURLcode res, Bytestream data);
    void cancelJobFinished(CURLcode res, Bytestream data);
    void identifyFinished(CURLcode res, Bytestream data);

    void getStringsFinished(CURLcode res, Bytestream data);
    void getImageFinished(CURLcode res, Bytestream data);

    void convertFailed(QString message);

private:
    QUrl _url;
    QUrl httpUrl();
    void resolveUrl(QUrl& url);

    QJsonObject opAttrs();

    void adjustRasterSettings(QString documentFormat, QJsonObject& jobAttrs, PrintParameters& Params);

    void setBusyMessage(QString msg);
    void setProgress(qint64 sent, qint64 total);

    bool isAllowedAddress(QUrl addr);

    QJsonValue getAttrOrDefault(QJsonObject jobAttrs, QString name, QString subkey = "");

    IppMsg mk_msg(QJsonObject opAttrs, QJsonObject jobAttrs=QJsonObject());

    QJsonObject _attrs;
    QJsonObject _jobAttrs;
    QJsonArray _jobs;

    QJsonObject _strings;
    QImage _icon;

    QStringList _additionalDocumentFormats;

    QString _busyMessage;
    QString _progress;

    QThread _workerThread;
    PrinterWorker _worker;

    bool _iconRetried = false;
};

#endif // IPPPRINTER_H
