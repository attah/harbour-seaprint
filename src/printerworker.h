#ifndef PRINTERWORKER_H
#define PRINTERWORKER_H
#include <QObject>
#include <QtNetwork>
#include "curlrequester.h"

class IppPrinter;

class ConvertFailedException: public std::exception
{
private:
    QString _msg;
public:
    ConvertFailedException(QString msg = "") : _msg(msg = msg)
    {
    }
    virtual const char* what() const throw()
    {
        return _msg.toStdString().c_str();
    }
};

class PrinterWorker : public QObject
{
    Q_OBJECT

public:
    PrinterWorker(IppPrinter* parent);

private:
    PrinterWorker();

public slots:
    void getPrinterAttributes(Bytestream msg);
    void getJobs(Bytestream msg);
    void cancelJob(Bytestream msg);

    void justUpload(QString filename, Bytestream header);

    void convertPdf(QString filename, Bytestream header,
                    QString targetFormat, quint32 Colors, quint32 Quality, QString PaperSize,
                    quint32 HwResX, quint32 HwResY, bool TwoSided, bool Tumble,
                    quint32 PageRangeLow, quint32 PageRangeHigh, bool BackHFlip, bool BackVFlip);

    void convertImage(QString filename, Bytestream header,
                      QString targetFormat, quint32 Colors, quint32 Quality, QString PaperSize,
                      quint32 HwResX, quint32 HwResY, QMargins margins);

    void convertOfficeDocument(QString filename, Bytestream header,
                               QString targetFormat, quint32 Colors, quint32 Quality, QString PaperSize,
                               quint32 HwResX, quint32 HwResY, bool TwoSided, bool Tumble,
                               quint32 PageRangeLow, quint32 PageRangeHigh, bool BackHFlip, bool BackVFlip);

    void convertPlaintext(QString filename, Bytestream header,
                          QString targetFormat, quint32 Colors, quint32 Quality, QString PaperSize,
                          quint32 HwResX, quint32 HwResY, bool TwoSided, bool Tumble, bool BackHFlip, bool BackVFlip);

signals:
    void done(QNetworkRequest request, QTemporaryFile* data);
    void progress(qint64 done, qint64 pages);
    void busyMessage(QString msg);
    void failed(QString message);

private:

    IppPrinter* _printer;

};

#endif // PRINTERWORKER_H
