#include <poppler/glib/poppler.h>
#include <poppler/glib/poppler-document.h>

#include <dlfcn.h>

#include "convertchecker.h"
#include <QProcess>
#include <QtDebug>
#include "mimer.h"

ConvertChecker::ConvertChecker()
{
    _pdf = true;
    _calligra = false;

    QProcess calligraconverter(this);
    calligraconverter.setProgram("calligraconverter");
    calligraconverter.setArguments({"-h"});
    calligraconverter.start();

    if(calligraconverter.waitForFinished(2000))
    {
      if(calligraconverter.exitStatus() == QProcess::NormalExit && calligraconverter.exitCode() == 0)
      {
          _calligra = true;
      }
    }
}

ConvertChecker::~ConvertChecker() {

}

ConvertChecker* ConvertChecker::m_Instance = 0;

ConvertChecker* ConvertChecker::instance()
{
    static QMutex mutex;
    if (!m_Instance)
    {
        mutex.lock();

        if (!m_Instance)
            m_Instance = new ConvertChecker;

        mutex.unlock();
    }

    return m_Instance;
}

quint32 ConvertChecker::pdfPages(QString filename)
{
    std::string url("file://");
    url.append(filename.toStdString());
    GError* error = nullptr;
    PopplerDocument* doc = poppler_document_new_from_file(url.c_str(), nullptr, &error);
    quint32 pages = poppler_document_get_n_pages(doc);
    g_object_unref(doc);

    return pages;
}
