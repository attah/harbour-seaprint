#ifndef CONVERTWORKER_H
#define CONVERTWORKER_H
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

class ConvertWorker : public QObject
{
    Q_OBJECT

public:
    ConvertWorker(IppPrinter* parent);

private:
    ConvertWorker();

public slots:
    void command(QByteArray msg);
    void getJobs(QByteArray msg);
    void cancelJob(QByteArray msg);

    void justUpload(QString filename, QByteArray header);

    void convertPdf(QString filename, QByteArray header,
                    QString targetFormat, quint32 Colors, quint32 Quality, QString PaperSize,
                    quint32 HwResX, quint32 HwResY, bool TwoSided, bool Tumble,
                    quint32 PageRangeLow, quint32 PageRangeHigh, bool BackHFlip, bool BackVFlip);

    void convertImage(QString filename, QByteArray header,
                      QString targetFormat, quint32 Colors, quint32 Quality, QString PaperSize,
                      quint32 HwResX, quint32 HwResY, QMargins margins);

    void convertOfficeDocument(QString filename, QByteArray header,
                               QString targetFormat, quint32 Colors, quint32 Quality, QString PaperSize,
                               quint32 HwResX, quint32 HwResY, bool TwoSided, bool Tumble,
                               quint32 PageRangeLow, quint32 PageRangeHigh, bool BackHFlip, bool BackVFlip);

    void convertPlaintext(QString filename, QByteArray header,
                          QString targetFormat, quint32 Colors, quint32 Quality, QString PaperSize,
                          quint32 HwResX, quint32 HwResY, bool TwoSided, bool Tumble, bool BackHFlip, bool BackVFlip);

signals:
    void done(QNetworkRequest request, QTemporaryFile* data);
    void progress(qint64 done, qint64 pages);
    void failed(QString message);

private:

    IppPrinter* _printer;

    QString getPopplerShortPaperSize(QString PaperSize);

    void adjustPageRange(QString PaperSize, quint32 PageRangeLow, quint32 PageRangeHigh,
                         QString pdfFileName, QTemporaryFile* tempfile);

    void pdftoPs(QString PaperSize, bool TwoSided, quint32 PageRangeLow, quint32 PageRangeHigh,
                 QString pdfFileName, QTemporaryFile* tempfile);

    void pdfToRaster(QString targetFormat, quint32 Colors, quint32 Quality, QString PaperSize,
                     quint32 HwResX, quint32 HwResY,  bool TwoSided, bool Tumble,
                     quint32 PageRangeLow, quint32 PageRangeHigh, quint32 pages,
                     bool BackHFlip, bool BackVFlip,
                     QString pdfFileName, QTemporaryFile* tempfile, bool resize);
};

#endif // CONVERTWORKER_H
