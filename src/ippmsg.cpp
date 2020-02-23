#include "ippmsg.h"

#define MAJ_VSN 1
#define MIN_VSN 1

quint32 IppMsg::_reqid=1;

IppMsg::IppMsg()
{
}

IppMsg::IppMsg(QJsonObject opAttrs, QJsonObject jobAttrs)
{
    _opAttrs = opAttrs;
    _jobAttrs = QJsonArray {jobAttrs};
}


IppMsg::~IppMsg()
{
}

IppMsg::IppMsg(QNetworkReply* resp)
{
    QByteArray tmp = resp->readAll();
    Bytestream bts(tmp.constData(), tmp.length());

    quint8 majVsn;
    quint8 minVsn;
    quint32 reqId;

    bts >> majVsn >> minVsn >> _status >> reqId;

    QJsonObject attrs;
    IppMsg::IppTag currentAttrType = IppTag::EndAttrs;

    QString last_name;

    while(!bts.atEnd())
    {
        if(bts.peekU8() <= IppTag::PrinterAttrs) {

            if(currentAttrType == IppTag::OpAttrs) {
                _opAttrs = attrs;
            }
            else if (currentAttrType == IppTag::JobAttrs) {
                _jobAttrs.append(attrs);
            }
            else if (currentAttrType == IppTag::PrinterAttrs) {
                _printerAttrs = attrs;
            }

            if(bts >>= (uint8_t)IppTag::EndAttrs) {
                break;
            }

            currentAttrType = (IppTag)bts.getU8();
            attrs = QJsonObject();

        }
        else {
            last_name = consume_attribute(attrs, bts, last_name);
        }
    }
}

QString IppMsg::consume_attribute(QJsonObject& attrs, Bytestream& data, QString lastName)
{
    quint8 tag;
    quint16 tmp_len;
    QString name;
    QJsonValue value;
    std::string tmp_str = "";
    bool noList = false;

    data >> tag >> tmp_len;

    data/tmp_len >> tmp_str;
    name = tmp_str!="" ? tmp_str.c_str() : lastName;


    switch (tag) {
        case OpAttrs:
        case JobAttrs:
        case EndAttrs:
        case PrinterAttrs:
            Q_ASSERT(false);
        case Integer:
        case Enum:
            quint32 tmp_u32;
            data >> tmp_len >> tmp_u32;
            value = (int)tmp_u32;
            break;
        case Boolean:
            quint8 tmp_bool;
            data >> tmp_len >> tmp_bool;
            value = (bool)tmp_bool;
            noList = true;
            break;
        case DateTime:
        {
            quint16 year;
            quint8 month, day, hour, minutes, seconds, deci_seconds,
                   plus_minus, utc_h_offset, utc_m_offset;
            data >> tmp_len >> year >> month >> day >> hour >> minutes >> seconds >> deci_seconds
                 >> plus_minus >> utc_h_offset >> utc_m_offset;
            QDate date(year, month, day);
            QTime time(hour, minutes, seconds, deci_seconds*100);
            int offset_seconds = (plus_minus == '+' ? 1 : -1)*(utc_h_offset*60*60+utc_m_offset*60);
            value = QDateTime(date, time, Qt::OffsetFromUTC, offset_seconds).toString(Qt::ISODate);
            break;
        }
        case Resolution:
        {
            qint32 x, y;
            qint8 units;
            QJsonObject tmp_res;
            data >> tmp_len >> x >> y >> units;
            tmp_res.insert("x", x);
            tmp_res.insert("y", y);
            tmp_res.insert("units", units);
            value = tmp_res;
            break;
        }
        case IntegerRange:
        {
            qint32 low, high;
            data >> tmp_len >> low >> high;
            QJsonObject tmp_range;
            tmp_range.insert("low", low);
            tmp_range.insert("high", high);
            value = tmp_range;
            noList = true;
            break;
        }
        case OctetStringUnknown:
        case TextWithLanguage:
        case NameWithLanguage:
        case TextWithoutLanguage:
        case NameWithoutLanguage:
        case Keyword:
        case Uri:
        case UriScheme:
        case Charset:
        case NaturalLanguage:
        case MimeMediaType:
        default:
            data >> tmp_len;
            data/tmp_len >> tmp_str;
            value = tmp_str.c_str();
            break;
    };


    if(attrs.contains(name))
    {
        QJsonObject tmp = attrs[name].toObject();
        QJsonArray tmpa;
        if(tmp["value"].isArray())
        {
            tmpa = tmp["value"].toArray();
        }
        else
        {
            tmpa = QJsonArray {tmp["value"]};
        }
        tmpa.append(value);
        tmp["value"] = tmpa;
        attrs.insert(name, tmp);
    }
    else
    {
        if((name.endsWith("-supported") || name == "printer-icons") && !noList)
        {
            value = QJsonArray {value};
        }
        attrs.insert(name, QJsonObject {{"tag", tag}, {"value", value}});
    }
    return name;
}

QByteArray IppMsg::encode(Operation op)
{
    Bytestream ipp;

    ipp << quint8(MAJ_VSN) << quint8(MIN_VSN);

    ipp << quint16(op);
    ipp << _reqid++;

    if(!_opAttrs.empty())
    {
        ipp << quint8(1);
        for(QJsonObject::iterator it = _opAttrs.begin(); it != _opAttrs.end(); it++)
        {
            QJsonObject val = it.value().toObject();
            ipp << encode_attr(val["tag"].toInt(), it.key(), val["value"]);
        }
    }
    for(QJsonArray::iterator ait = _jobAttrs.begin(); ait != _jobAttrs.begin(); ait++)
    {
        ipp << quint8(2);
        QJsonObject tmpObj = ait->toObject();
        for(QJsonObject::iterator it = tmpObj.begin(); it != tmpObj.end(); it++)
        {
            QJsonObject val = it.value().toObject();
            ipp << encode_attr(val["tag"].toInt(), it.key(), val["value"]);
        }
    }

    ipp << quint8(3);

    return QByteArray((char*)(ipp.raw()), ipp.size());
}

Bytestream IppMsg::encode_attr(quint8 tag, QString name, QJsonValueRef value)
{
    Bytestream req;

    switch (tag) {
        case OpAttrs:
        case JobAttrs:
        case EndAttrs:
        case PrinterAttrs:
            Q_ASSERT(false);
        case Integer:
        case Enum:
        {
            quint32 tmp_u32 = value.toInt();
            req << (quint16)4 << tmp_u32;
            break;
        }
        case Boolean:
        {
            quint32 tmp_u8 = value.toBool();
            req << (quint16)1 << tmp_u8;
            break;
        }
        case DateTime:
        {
            Q_ASSERT("fixme");
            break;
        }
        case Resolution:
        {
            qDebug() << value << value.toObject();
            qint32 x = value.toObject()["x"].toInt();
            qint32 y = value.toObject()["y"].toInt();
            qint8 units = value.toObject()["units"].toInt();
            req << (quint16)9 << x << y << units;
            break;
        }
        case IntegerRange:
        {
            qint32 low = value.toObject()["low"].toInt();
            qint32 high = value.toObject()["high"].toInt();
            req << (quint16)8 << low << high;
            break;
        }
        case OctetStringUnknown:
        case TextWithLanguage:
        case NameWithLanguage:
        case TextWithoutLanguage:
        case NameWithoutLanguage:
        case Keyword:
        case Uri:
        case UriScheme:
        case Charset:
        case NaturalLanguage:
        case MimeMediaType:
        {
            QByteArray tmpstr = value.toString().toUtf8();
            req << quint16(tmpstr.length());
            req.putBytes(tmpstr.data(), tmpstr.length());
            break;
        }
        default:
            qDebug() << "uncaught tag" << tag;
            Q_ASSERT(false);
            break;
    }

    Bytestream actual;
    if(req.size() != 0)
    {
        actual << tag << quint16(name.length()) << name.toStdString() << req;
    }

    return actual;
}
