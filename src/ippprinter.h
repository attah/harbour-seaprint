#ifndef IPPPRINTER_H
#define IPPPRINTER_H

#include <QtNetwork>
#include <QNetworkAccessManager>
#include "ippmsg.h"

class IppPrinter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString url READ getUrl WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QJsonObject attrs MEMBER _attrs NOTIFY attrsChanged)
    Q_PROPERTY(QJsonObject jobAttrs MEMBER _jobAttrs NOTIFY jobAttrsChanged)
    Q_PROPERTY(QJsonArray jobs MEMBER _jobs NOTIFY jobsChanged)


public:
    IppPrinter();
    ~IppPrinter();


    QString getUrl() {return _url.toString();}
    void setUrl(QString url);

    Q_INVOKABLE void refresh();

    Q_INVOKABLE bool getJobs();
    Q_INVOKABLE bool cancelJob(qint32 jobId);

signals:
    void urlChanged();
    void attrsChanged();
    void jobAttrsChanged();
    void jobsChanged();

    void jobFinished(bool status);
    void cancelStatus(bool status);

public slots:
    void print(QJsonObject attrs, QString file);


    void onUrlChanged();
    void getPrinterAttributesFinished(QNetworkReply* reply);
    void printRequestFinished(QNetworkReply* reply);
    void getJobsRequestFinished(QNetworkReply* reply);
    void cancelJobFinished(QNetworkReply* reply);

    void ignoreKnownSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);

private:
    QUrl _url;
    QUrl httpUrl();

    QJsonObject opAttrs();

    QNetworkAccessManager* _nam;
    QNetworkAccessManager* _jobs_nam;
    QNetworkAccessManager* _job_cancel_nam;
    QNetworkAccessManager* _print_nam;

    QJsonObject _attrs;
    QJsonObject _jobAttrs;
    QJsonArray _jobs;

};

#endif // IPPPRINTER_H
