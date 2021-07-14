#ifndef IPPPRINTER_H
#define IPPPRINTER_H

#include <QtNetwork>
#include <QNetworkAccessManager>
#include "ippmsg.h"
#include "convertworker.h"
#include <mlite5/MGConfItem>

class IppPrinter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString url READ getUrl WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QJsonObject attrs MEMBER _attrs NOTIFY attrsChanged)
    Q_PROPERTY(QJsonObject jobAttrs MEMBER _jobAttrs NOTIFY jobAttrsChanged)
    Q_PROPERTY(QJsonArray jobs MEMBER _jobs NOTIFY jobsChanged)
    Q_PROPERTY(QStringList additionalDocumentFormats MEMBER _additionalDocumentFormats NOTIFY additionalDocumentFormatsChanged)
    Q_PROPERTY(QString busyMessage MEMBER _busyMessage NOTIFY busyMessageChanged)
    Q_PROPERTY(QString progress MEMBER _progress NOTIFY progressChanged)

    Q_PROPERTY(bool tainted MEMBER _tainted NOTIFY taintedChanged)
    Q_PROPERTY(bool isIpps READ isIpps NOTIFY urlChanged)
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

    bool isIpps();
    bool correctSuffix();
    QStringList suffixes();

signals:
    void urlChanged();
    void attrsChanged();
    void jobAttrsChanged();
    void jobsChanged();

    void jobFinished(bool status);
    void cancelStatus(bool status);

    void doConvertPdf(QNetworkRequest request, QString filename, QTemporaryFile* tempfile,
                      QString targetFormat, quint32 Colors, quint32 Quality, QString PaperSize,
                      quint32 HwResX, quint32 HwResY, bool TwoSided, bool Tumble,
                      quint32 PageRangeLow, quint32 PageRangeHigh, bool BackHFlip, bool BackVFlip);

    void doConvertImage(QNetworkRequest request, QString filename,  QTemporaryFile* tempfile,
                        QString targetFormat, quint32 Colors, quint32 Quality, QString PaperSize,
                        quint32 HwResX, quint32 HwResY, QMargins margins);

    void doConvertOfficeDocument(QNetworkRequest request, QString filename, QTemporaryFile* tempfile,
                                 QString targetFormat, quint32 Colors, quint32 Quality, QString PaperSize,
                                 quint32 HwResX, quint32 HwResY, bool TwoSided, bool Tumble,
                                 quint32 PageRangeLow, quint32 PageRangeHigh, bool BackHFlip, bool BackVFlip);

    void doConvertPlaintext(QNetworkRequest request, QString filename, QTemporaryFile* tempfile,
                            QString targetFormat, quint32 Colors, quint32 Quality, QString PaperSize,
                            quint32 HwResX, quint32 HwResY, bool TwoSided, bool Tumble, bool BackHFlip, bool BackVFlip);

    void additionalDocumentFormatsChanged();
    void busyMessageChanged();
    void progressChanged();
    void taintedChanged();

public slots:
    void print(QJsonObject attrs, QString file);


    void onUrlChanged();
    void UpdateAdditionalDocumentFormats();
    void getPrinterAttributesFinished(QNetworkReply* reply);
    void printRequestFinished(QNetworkReply* reply);
    void getJobsRequestFinished(QNetworkReply* reply);
    void cancelJobFinished(QNetworkReply* reply);

    void onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
    static void ignoreSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);

    void convertDone(QNetworkRequest request, QTemporaryFile* data);
    void convertFailed(QString message);

private:
    QUrl _url;
    QUrl httpUrl();
    QNetworkRequest mkReq();

    QJsonObject opAttrs();

    void adjustRasterSettings(QString documentFormat, QJsonObject& jobAttrs, quint32& HwResX, quint32& HwResY,
                              bool& BackHFlip, bool& BackVFlip);

    void setBusyMessage(QString msg);
    void setProgress(qint64 sent, qint64 total);

    QJsonValue getAttrOrDefault(QJsonObject jobAttrs, QString name, QString subkey = "");

    IppMsg mk_msg(QJsonObject opAttrs, QJsonObject jobAttrs=QJsonObject());

    QNetworkAccessManager* _nam;
    QNetworkAccessManager* _jobs_nam;
    QNetworkAccessManager* _job_cancel_nam;
    QNetworkAccessManager* _print_nam;

    QJsonObject _attrs;
    QJsonObject _jobAttrs;
    QJsonArray _jobs;

    QStringList _additionalDocumentFormats;

    QString _busyMessage;
    QString _progress;

    QThread _workerThread;
    ConvertWorker* _worker;

    bool _tainted;
};

#endif // IPPPRINTER_H
