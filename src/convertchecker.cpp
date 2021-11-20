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

quint32 ConvertChecker::pdfPages(QString /*filename*/)
{
    return 0;
}
