#ifndef PRINTERWORKER_H
#define PRINTERWORKER_H
#include <QObject>
#include "curlrequester.h"
#include "ppm2pwg/printparameters.h"

class IppPrinter;

class ConvertFailedException: public std::exception
{
private:
    std::string _msg;
public:
    ConvertFailedException(QString msg = "") : _msg(msg.toStdString())
    {
    }
    virtual const char* what() const throw()
    {
        return _msg.c_str();
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
    void getStrings(QUrl url);
    void getImage(QUrl url);
    void getPrinterAttributes(Bytestream msg);
    void getJobs(Bytestream msg);
    void cancelJob(Bytestream msg);
    void identify(Bytestream msg);

    void justUpload(QString filename, Bytestream header);

    void fixupImage(QString filename, Bytestream header, QString targetFormat);

    void fixupPlaintext(QString filename, Bytestream header);

    void convertPdf(QString filename, Bytestream header, PrintParameters Params);

    void convertImage(QString filename, Bytestream header, PrintParameters Params, QMargins margins);

    void convertOfficeDocument(QString filename, Bytestream header, PrintParameters Params);

    void convertPlaintext(QString filename, Bytestream header, PrintParameters Params);

signals:
    void progress(qint64 done, qint64 pages);
    void busyMessage(QString msg);
    void failed(QString message);

private:

    IppPrinter* _printer;

};

#endif // PRINTERWORKER_H
