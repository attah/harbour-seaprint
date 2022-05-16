#ifndef MIMER_H
#define MIMER_H

#include <QString>
#include <QObject>
#include <QMutex>
#include <QMimeDatabase>

class Mimer : public  QObject
{
    Q_OBJECT
public:
    static Mimer* instance();
    Q_INVOKABLE QString get_type(QString filename);

    Q_PROPERTY(const QString OctetStream MEMBER OctetStream CONSTANT);

    Q_PROPERTY(const QString PDF MEMBER PDF CONSTANT);
    Q_PROPERTY(const QString Postscript MEMBER Postscript CONSTANT);
    Q_PROPERTY(const QString PWG MEMBER PWG CONSTANT);
    Q_PROPERTY(const QString URF MEMBER URF CONSTANT);

    Q_PROPERTY(const QString PNG MEMBER PNG CONSTANT);
    Q_PROPERTY(const QString GIF MEMBER GIF CONSTANT);
    Q_PROPERTY(const QString JPEG MEMBER JPEG CONSTANT);
    Q_PROPERTY(const QString TIFF MEMBER TIFF CONSTANT);

    Q_PROPERTY(const QString DOC MEMBER DOC CONSTANT);
    Q_PROPERTY(const QString DOCX MEMBER DOCX CONSTANT);
    Q_PROPERTY(const QString RTF MEMBER RTF CONSTANT);
    Q_PROPERTY(const QString ODT MEMBER ODT CONSTANT);
    Q_PROPERTY(const QString PPT MEMBER PPT CONSTANT);
    Q_PROPERTY(const QString PPTX MEMBER PPTX CONSTANT);
    Q_PROPERTY(const QString ODP MEMBER ODP CONSTANT);


    Q_PROPERTY(const QString Plaintext MEMBER Plaintext CONSTANT);

    Q_PROPERTY(const QStringList OfficeFormats MEMBER OfficeFormats CONSTANT);

    static const QString OctetStream;

    static const QString PDF;
    static const QString Postscript;
    static const QString PWG;
    static const QString URF;

    static const QString PNG;
    static const QString GIF;
    static const QString JPEG;
    static const QString TIFF;

    static const QString DOC;
    static const QString DOCX;
    static const QString RTF;
    static const QString RTF_APP;
    static const QString ODT;
    static const QString PPT;
    static const QString PPTX;
    static const QString ODP;

    static const QString Plaintext;

    static const QStringList RasterFormats;
    static const QStringList OfficeFormats;

    Q_INVOKABLE static bool isImage(QString mimeType)
    {
        return mimeType.startsWith("image/") && ! isRaster(mimeType);
    }
    Q_INVOKABLE static bool isRaster(QString mimeType)
    {
        return RasterFormats.contains(mimeType);
    }
    Q_INVOKABLE static bool isOffice(QString mimeType)
    {
        return OfficeFormats.contains(mimeType);
    }

private:
    Mimer();
    static Mimer* m_Instance;
    QMimeDatabase _db;

};

#endif // MIMER_H
