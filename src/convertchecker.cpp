#include "convertchecker.h"
#include <QProcess>
#include <QtDebug>

ConvertChecker::ConvertChecker()
{
    _pdf = false;
    QProcess* pdftoppm = new QProcess(this);
    pdftoppm->setProgram("pdftoppm");
    pdftoppm->setArguments({"-h"});
    pdftoppm->start();
    if(pdftoppm->waitForFinished(2000))
    {
      if(pdftoppm->exitStatus() == QProcess::NormalExit && pdftoppm->exitCode() == 0)
      {
          _pdf = true;
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
    quint32 pages = 0;
    if(!_pdf)
    {
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
