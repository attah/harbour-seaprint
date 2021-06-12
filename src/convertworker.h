#ifndef CONVERTWORKER_H
#define CONVERTWORKER_H
#include <QObject>
#include <QtNetwork>

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

public slots:
    void convertPdf(QNetworkRequest request, QString filename, QTemporaryFile* tempfile,
                    QString targetFormat, quint32 Colors, quint32 Quality, QString PaperSize,
                    quint32 HwResX, quint32 HwResY, bool TwoSided, bool Tumble,
                    quint32 PageRangeLow, quint32 PageRangeHigh);

    void convertImage(QNetworkRequest request, QString filename, QTemporaryFile* tempfile,
                      QString targetFormat, quint32 Colors, quint32 Quality, QString PaperSize,
                      quint32 HwResX, quint32 HwResY);

    void convertOfficeDocument(QNetworkRequest request, QString filename, QTemporaryFile* tempfile,
                               QString targetFormat, quint32 Colors, quint32 Quality, QString PaperSize,
                               quint32 HwResX, quint32 HwResY, bool TwoSided, bool Tumble,
                               quint32 PageRangeLow, quint32 PageRangeHigh);

signals:
    void done(QNetworkRequest request, QTemporaryFile* data);
    void progress(qint64 done, qint64 pages);
    void failed(QString message);

private:

    QString getPopplerShortPaperSize(QString PaperSize);

    void pdftoPs(QString PaperSize, bool TwoSided, quint32 PageRangeLow, quint32 PageRangeHigh,
                 QString pdfFileName, QTemporaryFile* tempfile);

    void pdfToRaster(QString targetFormat, quint32 Colors, quint32 Quality, QString PaperSize,
                     quint32 HwResX, quint32 HwResY,  bool TwoSided, bool Tumble,
                     quint32 PageRangeLow, quint32 PageRangeHigh, quint32 pages,
                     QString pdfFileName, QTemporaryFile* tempfile, bool resize);
};

#endif // CONVERTWORKER_H
