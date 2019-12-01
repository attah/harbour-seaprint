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

public:
    IppPrinter();
    ~IppPrinter();


    QString getUrl() {return _url;}
    void setUrl(QString url);

    Q_INVOKABLE bool print(QJsonObject attrs, QString file);

signals:
    void urlChanged();
    void attrsChanged();
    void jobAttrsChanged();

public slots:
    void onUrlChanged();
    void getPrinterAttributesFinished(QNetworkReply* reply);
    void jobRequestFinished(QNetworkReply* reply);

private:
    QString _url;

    QNetworkAccessManager* _nam;
    QNetworkAccessManager* _jnam;

    QJsonObject _attrs;
    QJsonObject _jobAttrs;

};

#endif // IPPPRINTER_H
