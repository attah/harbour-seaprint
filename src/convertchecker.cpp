#include "convertchecker.h"
#include <QProcess>

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
