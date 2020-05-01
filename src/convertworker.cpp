#include "convertworker.h"

void ConvertWorker::convertPdf(QNetworkRequest request, QString filename, QTemporaryFile* tempfile)
{

    QProcess* muraster = new QProcess(this);
    muraster->setProgram("/home/nemo/stuff/bin/muraster");
    muraster->setArguments({"-F", "pgm", filename});


    QProcess* ppm2pwg = new QProcess(this);
    ppm2pwg->setProgram("/home/nemo/repos/pwg/ppm2pwg");
    QStringList env; // {"PREPEND_FILE="+tempfile->fileName()};

    bool apple = false;
    if(apple)
    {
        env.append("URF=true");
    }

    qDebug() << "Prepend file env done";

    ppm2pwg->setEnvironment(env);

    muraster->setStandardOutputProcess(ppm2pwg);
    ppm2pwg->setStandardOutputFile(tempfile->fileName(), QIODevice::Append);

    connect(muraster, SIGNAL(finished(int, QProcess::ExitStatus)), muraster, SLOT(deleteLater()));
    connect(ppm2pwg, SIGNAL(finished(int, QProcess::ExitStatus)), ppm2pwg, SLOT(deleteLater()));

    qDebug() << "All connected";


    muraster->start();
    ppm2pwg->start();

    qDebug() << "Starting";


    if(!muraster->waitForStarted())
    {
        qDebug() << "muraster died";
        tempfile->deleteLater();
        emit failed();
        return;
    }
    if(!ppm2pwg->waitForStarted())
    {
        qDebug() << "ppm2pwg died";
        tempfile->deleteLater();
        emit failed();
        return;
    }
    qDebug() << "All started";

    ppm2pwg->waitForFinished();

    qDebug() << "Finished";

    emit done(request, tempfile);
    qDebug() << "posted";
}
