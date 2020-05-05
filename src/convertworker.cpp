#include "convertworker.h"
#include <sailfishapp.h>

void ConvertWorker::convertPdf(QNetworkRequest request, QString filename,
                               bool apple, quint32 HwResX, quint32 HwResY, QTemporaryFile* tempfile)
{

    QString HwResX_s = QString::number(HwResX);
    QString HwResY_s = QString::number(HwResY);

    QProcess* pdftoppm = new QProcess(this);
    pdftoppm->setProgram("pdftoppm");
    pdftoppm->setArguments({"-gray", "-rx", HwResX_s, "-ry", HwResY_s, filename});


    QProcess* ppm2pwg = new QProcess(this);
    // Yo dwag, I heard you like programs...
    ppm2pwg->setProgram("harbour-seaprint");
    ppm2pwg->setArguments({"ppm2pwg"});
    QStringList env = {"HWRES_X="+HwResX_s, "HWRES_Y="+HwResY_s};

    if(apple)
    {
        env.append("URF=true");
    }

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
