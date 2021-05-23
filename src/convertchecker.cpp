#include "convertchecker.h"
#include <QProcess>
#include <QtDebug>
#include "mimer.h"

ConvertChecker::ConvertChecker()
{
    _pdf = false;
    _calligra = false;
    QProcess* pdftoppm = new QProcess(this);
    pdftoppm->setProgram("pdftoppm");
    pdftoppm->setArguments({"-h"});
    pdftoppm->start();

    QProcess* calligraconverter = new QProcess(this);
    calligraconverter->setProgram("calligraconverter");
    calligraconverter->setArguments({"-h"});
    calligraconverter->start();

    if(pdftoppm->waitForFinished(2000))
    {
      if(pdftoppm->exitStatus() == QProcess::NormalExit && pdftoppm->exitCode() == 0)
      {
          _pdf = true;
      }
    }

    if(calligraconverter->waitForFinished(2000))
    {
      if(calligraconverter->exitStatus() == QProcess::NormalExit && calligraconverter->exitCode() == 0)
      {
          _calligra = true;
      }
    }

    pdftoppm->deleteLater();
    calligraconverter->deleteLater();
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
    quint32 pages = 0;
    if(!_pdf || (Mimer::instance()->get_type(filename) != Mimer::PDF))
    { // pdfinfo is a bit slow to return on some non-PDFs
        return pages;
    }

    QProcess* pdfinfo = new QProcess(this);
    pdfinfo->setProgram("pdfinfo");
    pdfinfo->setArguments({filename});
    pdfinfo->start();

    if(!pdfinfo->waitForStarted(1000) || !pdfinfo->waitForFinished(1000))
    {
        pdfinfo->deleteLater();
        return pages;
    }
    QByteArray pdfInfoOutput = pdfinfo->readAllStandardOutput();
    pdfinfo->deleteLater();
    qDebug() << pdfInfoOutput;
    QList<QByteArray> pdfInfoOutputLines = pdfInfoOutput.split('\n');
    for(QList<QByteArray>::iterator it = pdfInfoOutputLines.begin(); it != pdfInfoOutputLines.end(); it++)
    {
        if(it->startsWith("Pages"))
        {
            QList<QByteArray> pagesTokens = it->split(' ');
            pages = pagesTokens.last().toInt();
        }
    }
    return pages;
}
