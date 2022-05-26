#include "mimer.h"

const QString Mimer::OctetStream = "application/octet-stream";

const QString Mimer::PDF = "application/pdf";
const QString Mimer::Postscript = "application/postscript";
const QString Mimer::PWG = "image/pwg-raster";
const QString Mimer::URF = "image/urf";

const QString Mimer::PNG = "image/png";
const QString Mimer::GIF = "image/gif";
const QString Mimer::JPEG = "image/jpeg";
const QString Mimer::TIFF = "image/tiff";
const QString Mimer::RBMP = "image/reverse-encoding-bmp";

const QString Mimer::DOC = "application/msword";
const QString Mimer::DOCX = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
const QString Mimer::RTF = "text/rtf";
const QString Mimer::RTF_APP = "application/rtf";
const QString Mimer::ODT = "application/vnd.oasis.opendocument.text";
const QString Mimer::PPT = "application/vnd.ms-powerpoint";
const QString Mimer::PPTX = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
const QString Mimer::ODP = "application/vnd.oasis.opendocument.presentation";

const QString Mimer::Plaintext = "text/plain";

const QStringList Mimer::RasterFormats = {PWG, URF};
const QStringList Mimer::OfficeFormats = {DOC, DOCX, RTF, RTF_APP, ODT, PPT, PPTX, ODP};

Mimer::Mimer()
{

}

Mimer* Mimer::m_Instance = 0;

Mimer* Mimer::instance()
{
    static QMutex mutex;
    if (!m_Instance)
    {
        mutex.lock();

        if (!m_Instance)
            m_Instance = new Mimer;

        mutex.unlock();
    }

    return m_Instance;
}

QString Mimer::get_type(QString filename) {
    return _db.mimeTypeForFile(filename).name();
}
