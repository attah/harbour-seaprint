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
