#include "ippmsg.h"

quint32 IppMsg::_reqid=1;

IppMsg::IppMsg()
{
}

IppMsg::IppMsg(QJsonObject opAttrs, QJsonObject jobAttrs, quint8 majVsn, quint8 minVsn)
{
    _majVsn = majVsn;
    _minVsn = minVsn;
    _opAttrs = opAttrs;
    _jobAttrs = QJsonArray {jobAttrs};
}


IppMsg::~IppMsg()
{
}


IppMsg::IppMsg(Bytestream& bts)
{
//    Bytestream bts(resp.constData(), resp.length());

    quint32 reqId;

    bts >> _majVsn >> _minVsn >> _status >> reqId;

    QJsonObject attrs;
    IppMsg::IppTag currentAttrType = IppTag::EndAttrs;

    while(!bts.atEnd())
    {
        if(bts.peekU8() <= IppTag::UnsupportedAttrs) {

            if(currentAttrType == IppTag::OpAttrs) {
                _opAttrs = attrs;
            }
            else if (currentAttrType == IppTag::JobAttrs) {
                _jobAttrs.append(attrs);
            }
            else if (currentAttrType == IppTag::PrinterAttrs) {
                _printerAttrs = attrs;
            }
            else if (currentAttrType == IppTag::UnsupportedAttrs) {
                qDebug() << "WARNING: unsupported attrs reported:" << attrs;
            }

            if(bts >>= (uint8_t)IppTag::EndAttrs) {
                break;
            }

            currentAttrType = (IppTag)bts.getU8();
            attrs = QJsonObject();

        }
        else {
            consume_attribute(attrs, bts);
        }
    }
}

QJsonValue IppMsg::consume_value(quint8 tag, Bytestream& data)
{
    QJsonValue value;
    quint16 tmp_len;

    switch (tag) {
        case OpAttrs:
        case JobAttrs:
        case EndAttrs:
        case PrinterAttrs:
        case UnsupportedAttrs:
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
            data >> tmp_len >> x >> y >> units;
            value = QJsonObject {{"x", x}, {"y", y}, {"units", units}};
            break;
        }
        case IntegerRange:
        {
            qint32 low, high;
            data >> tmp_len >> low >> high;
            value = QJsonObject {{"low", low}, {"high", high}};
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
        {
            std::string tmp_str = "";
            data >> tmp_len;
            data/tmp_len >> tmp_str;
            value = tmp_str.c_str();
            break;
        }
    };
    return QJsonObject {{"tag", tag}, {"value", value}};
}

QJsonArray IppMsg::get_unnamed_attributes(Bytestream& data)
{
    quint8 tag;
    QJsonArray attrs;
    while(data.remaining())
    {
        data >> tag;
        if(data >>= (quint16)0)
        {
            attrs.append(consume_value(tag, data));
        }
        else
        {
            data -= 1;
            break;
        }
    }
    return attrs;
}

QJsonArray untag_values(QJsonArray taggedValues)
{
    QJsonArray res;
    for(QJsonValue it : taggedValues)
    {
        res.append(it.toObject()["value"]);
    }
    return res;
}

QJsonValue IppMsg::collect_attributes(QJsonArray& attrs)
{
    QJsonArray resArr;
    QJsonObject resObj = QJsonObject();
    while(!attrs.empty())
    {
        QJsonObject tmpobj = attrs.takeAt(0).toObject();
        quint8 tag = tmpobj["tag"].toInt();
        if(tag == MemberName)
        {
            QString key = tmpobj["value"].toString();
            tmpobj = attrs.takeAt(0).toObject();
            if(tmpobj["tag"] == BeginCollection)
            {
                resObj[key] = QJsonObject {{"tag", BeginCollection}, {"value", collect_attributes(attrs)}};
            }
            else
            { // This should be general data attributes
                QJsonArray restOfSet;
                while(attrs.begin()->toObject()["tag"] == tmpobj["tag"])
                {
                    restOfSet.append(attrs.takeAt(0));
                }

                if(restOfSet.empty())
                {
                    resObj[key] = tmpobj;
                }
                else
                {
                    restOfSet.prepend(tmpobj);
                    tmpobj["value"] = untag_values(restOfSet);
                    resObj[key] = tmpobj;
                }
            }
        }
        else if(tag == EndCollection)
        {
            resArr.append(resObj);
            resObj = QJsonObject();
            if(attrs.empty())
            { // end of collection
                break;
            }
            else if(attrs.begin()->toObject()["tag"] == BeginCollection)
            { // this is a 1setOf
                attrs.pop_front();
                continue;
            }
            else
            { // the following attribute(s) belong to an outer collection
                break;
            }
        }
        else
        {
            qDebug() << "out of sync with collection" << tmpobj;
        }
    }

    if(resArr.size()==1)
    { // The code above unconditionally produces arrays, collapse if they are just a single object
        return resArr.first();
    }
    else
    {
        return resArr;
    }
}

QString IppMsg::consume_attribute(QJsonObject& attrs, Bytestream& data)
{
    quint8 tag;
    quint16 tmp_len;
    QString name;
    QJsonValue taggedValue;
    std::string tmp_str = "";

    data >> tag >> tmp_len;

    data/tmp_len >> tmp_str;
    QString name0 = tmp_str.c_str();
    name = tmp_str.c_str();

    taggedValue = consume_value(tag, data);

    QJsonArray unnamed = get_unnamed_attributes(data);

    if(tag == BeginCollection)
    {
        QJsonValue collected = collect_attributes(unnamed);
        taggedValue = QJsonObject {{"tag", tag}, {"value", collected}};
    }

    bool noList = (tag == Boolean || tag == IntegerRange);
    bool forceArray = (!noList && (name.startsWith("marker-")
                                   || name.startsWith("printer-firmware")
                                   || name.endsWith("-supported")
                                   || name == "printer-icons"
                                   || name.endsWith("-reasons")));

    if(!unnamed.empty() || forceArray)
    {
        unnamed.prepend(taggedValue);
        attrs.insert(name, QJsonObject {{"tag", tag}, {"value", untag_values(unnamed)}});

    }
    else
    {
        attrs.insert(name, taggedValue);
    }
    return name;
}

Bytestream IppMsg::encode(Operation op)
{
    Bytestream ipp;

    ipp << _majVsn << _minVsn;

    ipp << quint16(op);
    ipp << _reqid++;


    ipp << quint8(OpAttrs);
    // attributes-charset and attributes-natural-language are required to be first
    // some printers fail if the other mandatory parameters are not in this specific order
    QStringList InitialAttrs = {"attributes-charset",
                                "attributes-natural-language",
                                "printer-uri",
                                "requesting-user-name"};
    for(QString key : InitialAttrs)
    {
        QJsonObject val = _opAttrs.take(key).toObject();
        encode_attr(ipp, val["tag"].toInt(), key, val["value"]);
    }
    for(QJsonObject::iterator it = _opAttrs.begin(); it != _opAttrs.end(); it++)
    { // encode any remaining op-attrs
        QJsonObject val = it.value().toObject();
        encode_attr(ipp, val["tag"].toInt(), it.key(), val["value"]);
    }
    for(QJsonArray::iterator ait = _jobAttrs.begin(); ait != _jobAttrs.end(); ait++)
    {
        QJsonObject tmpObj = ait->toObject();
        if (!tmpObj.isEmpty()) {
            ipp << quint8(JobAttrs);
            for(QJsonObject::iterator it = tmpObj.begin(); it != tmpObj.end(); it++)
            {
                QJsonObject val = it.value().toObject();
                encode_attr(ipp, val["tag"].toInt(), it.key(), val["value"]);
            }
        }
    }

    ipp << quint8(EndAttrs);

    return ipp;
}

void IppMsg::encode_attr(Bytestream& msg, quint8 tag, QString name, QJsonValue value, bool inCollection)
{

    if(inCollection)
    {
        msg << (quint8)MemberName  << (quint16)0 << (quint16)name.length() << name.toStdString();
        name = "";
    }
    msg << tag << quint16(name.length()) << name.toStdString();


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
            msg << (quint16)4 << tmp_u32;
            break;
        }
        case Boolean:
        {
            quint32 tmp_u8 = value.toBool();
            msg << (quint16)1 << tmp_u8;
            break;
        }
        case DateTime:
        {
            Q_ASSERT("fixme");
            break;
        }
        case Resolution:
        {
            qint32 x = value.toObject()["x"].toInt();
            qint32 y = value.toObject()["y"].toInt();
            qint8 units = value.toObject()["units"].toInt();
            msg << (quint16)9 << x << y << units;
            break;
        }
        case IntegerRange:
        {
            qint32 low = value.toObject()["low"].toInt();
            qint32 high = value.toObject()["high"].toInt();
            msg << (quint16)8 << low << high;
            break;
        }
        case BeginCollection:
        {
            msg << (quint16)0; // length of value
            if(value.isObject())
            {
                QJsonObject collection = value.toObject();
                for(QString key : collection.keys())
                {
                    encode_attr(msg, collection[key].toObject()["tag"].toInt(), key,
                                collection[key].toObject()["value"], true);
                }
            }
            else
            {
                // TODO add support for 1-setOf in collections
                Q_ASSERT("FIXME-array");
            }
            msg << (quint8)EndCollection << (quint16)0 << (quint16)0;
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
            msg << quint16(tmpstr.length());
            msg.putBytes(tmpstr.data(), tmpstr.length());
            break;
        }
        default:
            qDebug() << "uncaught tag" << tag;
            Q_ASSERT(false);
            break;
    }
}
