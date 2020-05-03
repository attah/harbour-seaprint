#ifndef CONVERTWORKER_H
#define CONVERTWORKER_H
#include <QObject>
#include <QtNetwork>

class ConvertWorker : public QObject
{
    Q_OBJECT

public slots:
    void convertPdf(QNetworkRequest request, QString filename,
                    bool apple, QTemporaryFile* tempfile);
    //convertImage();

signals:
    void done(QNetworkRequest request, QTemporaryFile* data);
    void failed();
};

#endif // CONVERTWORKER_H
