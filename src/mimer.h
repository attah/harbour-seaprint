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

    static const QString OctetStream;

    static const QString PDF;
    static const QString Postscript;
    static const QString PWG;
    static const QString URF;

    static const QString PNG;
    static const QString GIF;
    static const QString JPEG;
    static const QString TIFF;

    Q_INVOKABLE static bool isImage(QString mimeType) {return mimeType.startsWith("image/");}


private:
    Mimer();
    static Mimer* m_Instance;
    QMimeDatabase _db;

};

#endif // MIMER_H
