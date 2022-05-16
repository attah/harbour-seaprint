#ifndef IPP_MSG_H
#define IPP_MSG_H

#include "bytestream.h"

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QDate>
#include <QTime>
#include <QDebug>

class IppMsg
{
    Q_GADGET
public:

    enum IppTag : quint8 {
        OpAttrs             = 0x01,
        JobAttrs            = 0x02,
        EndAttrs            = 0x03,
        PrinterAttrs        = 0x04,
        UnsupportedAttrs    = 0x05,
        Unsupported         = 0x10,
        Integer             = 0x21,
        Boolean             = 0x22,
        Enum                = 0x23,
        OctetStringUnknown  = 0x30,
        DateTime            = 0x31,
        Resolution          = 0x32,
        IntegerRange        = 0x33,
        BeginCollection     = 0x34,
        TextWithLanguage    = 0x35,
        NameWithLanguage    = 0x36,
        EndCollection       = 0x37,
        TextWithoutLanguage = 0x41,
        NameWithoutLanguage = 0x42,
        Keyword             = 0x44,
        Uri                 = 0x45,
        UriScheme           = 0x46,
        Charset             = 0x47,
        NaturalLanguage     = 0x48,
        MimeMediaType       = 0x49,
        MemberName          = 0x4A
    };
    Q_ENUMS(IppTag)

    enum Operation : quint16 {
        PrintJob          = 0x0002,
        PrintUri          = 0x0003,
        ValidateJob       = 0x0004,
        CreateJob         = 0x0005,
        SendDocument      = 0x0006,
        SendUri           = 0x0007,
        CancelJob         = 0x0008,
        GetJobAttrs       = 0x0009,
        GetJobs           = 0x000A,
        GetPrinterAttrs   = 0x000B,
        HoldJob           = 0x000C,
        ReleaseJob        = 0x000D,
        RestartJob        = 0x000E,
        PausePrinter      = 0x0010,
        ResumePrinter     = 0x0011,
        PurgeJobs         = 0x0012,
        IdentifyPrinter   = 0x003C
    };

    explicit IppMsg();
    explicit IppMsg(Bytestream& resp);
    IppMsg(QJsonObject opAttrs, QJsonObject jobAttrs=QJsonObject(), quint8 majVsn=1, quint8 minVsn=1);
    IppMsg(const IppMsg& other) = default;
    ~IppMsg();

    Bytestream encode(Operation op);
    QJsonObject getPrinterAttrs() {return _printerAttrs;}
    QJsonArray getJobAttrs() {return _jobAttrs;}
    QJsonObject getOpAttrs() {return _opAttrs;}

    quint16 getStatus() {return _status;}


protected:
private:
    QJsonValue consume_value(quint8 tag, Bytestream& data);
    QJsonArray get_unnamed_attributes(Bytestream& data);
    QJsonValue collect_attributes(QJsonArray& attrs);
    QString consume_attribute(QJsonObject& attrs, Bytestream& data);
    void encode_attr(Bytestream& msg, quint8 tag, QString name, QJsonValue value, bool subCollection=false);

    quint8 _majVsn;
    quint8 _minVsn;

    QJsonObject _opAttrs;
    QJsonArray _jobAttrs;
    QJsonObject _printerAttrs;

    quint16 _status;

    static quint32 _reqid;
};

#endif // IPP_MSG_H
