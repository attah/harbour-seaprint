#include "convertworker.h"
#include <sailfishapp.h>

void ConvertWorker::convertPdf(QNetworkRequest request, QString filename,
                               bool apple, QTemporaryFile* tempfile)
{

    QProcess* pdftoppm = new QProcess(this);
    pdftoppm->setProgram("pdftoppm");
    pdftoppm->setArguments({"-gray", "-rx", "300", "-ry", "300", filename});


    QProcess* ppm2pwg = new QProcess(this);
    // Yo dwag, I heard you like programs...
    ppm2pwg->setProgram("harbour-seaprint");
    ppm2pwg->setArguments({"ppm2pwg"});
    QStringList env; // {"PREPEND_FILE="+tempfile->fileName()};

    if(apple)
    {
        env.append("URF=true");
    }

    qDebug() << "Prepend file env done";

    ppm2pwg->setEnvironment(env);

    pdftoppm->setStandardOutputProcess(ppm2pwg);
    ppm2pwg->setStandardOutputFile(tempfile->fileName(), QIODevice::Append);

    connect(pdftoppm, SIGNAL(finished(int, QProcess::ExitStatus)), pdftoppm, SLOT(deleteLater()));
    connect(ppm2pwg, SIGNAL(finished(int, QProcess::ExitStatus)), ppm2pwg, SLOT(deleteLater()));

    qDebug() << "All connected";


    pdftoppm->start();
    ppm2pwg->start();

    qDebug() << "Starting";


    if(!pdftoppm->waitForStarted())
    {
        qDebug() << "pdftoppm died";
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
