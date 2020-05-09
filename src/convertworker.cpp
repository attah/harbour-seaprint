#include "convertworker.h"
#include <sailfishapp.h>

void ppm2PwgEnv(QStringList& env, bool urf, quint32 Quality,
                quint32 HwResX, quint32 HwResY, bool TwoSided, bool Tumble)
{
    env.append("HWRES_X="+QString::number(HwResX));
    env.append("HWRES_Y="+QString::number(HwResY));

    if(urf)
    {
        env.append("URF=true");
    }

    if(Quality >= 3 && Quality <= 5)
    {
        env.append("QUALITY="+QString::number(Quality));
    }

    env.append("DUPLEX="+QString::number(TwoSided));
    env.append("TUMBLE="+QString::number(Tumble));

}

void ConvertWorker::convertPdf(QNetworkRequest request, QString filename, QTemporaryFile* tempfile,
                               bool urf, quint32 Colors, quint32 Quality,
                               quint32 HwResX, quint32 HwResY, bool TwoSided, bool Tumble)
{
    if(urf)
    { // URF only supports symmetric resolutions
        if(HwResX < HwResY)
        {
            HwResY = HwResX;
        }
        else {
            HwResX = HwResY;
        }
    }

    QProcess* pdftoppm = new QProcess(this);
    pdftoppm->setProgram("pdftoppm");
    QStringList Pdf2PpmArgs = {"-rx", QString::number(HwResX), "-ry", QString::number(HwResY)};
    if(Colors != 3)
    {
        Pdf2PpmArgs.append("-gray");
    }
    Pdf2PpmArgs.append(filename);
    pdftoppm->setArguments(Pdf2PpmArgs);


    QProcess* ppm2pwg = new QProcess(this);
    // Yo dawg, I heard you like programs...
    ppm2pwg->setProgram("harbour-seaprint");
    ppm2pwg->setArguments({"ppm2pwg"});

    QStringList env;
    ppm2PwgEnv(env, urf, Quality, HwResX, HwResY, TwoSided, Tumble);
    qDebug() << "ppm2pwg env is " << env;

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
