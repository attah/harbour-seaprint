#include "convertworker.h"
#include <sailfishapp.h>
#include "papersizes.h"
#include <QImage>
#include <QMatrix>
#include <QPainter>

void ppm2PwgEnv(QStringList& env, bool urf, quint32 Quality, QString PaperSize,
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

    if(PaperSize != "")
    {
        env.append("PAGE_SIZE_NAME="+PaperSize);
    }

    env.append("DUPLEX="+QString::number(TwoSided));
    env.append("TUMBLE="+QString::number(Tumble));

}

void ConvertWorker::convertPdf(QNetworkRequest request, QString filename, QTemporaryFile* tempfile,
                               bool urf, quint32 Colors, quint32 Quality, QString PaperSize,
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

    QString ShortPaperSize;
    if(PaperSize == "iso_a4_210x297mm")
    {
        ShortPaperSize = "A4";
    }
    else if (PaperSize == "iso_a3_297x420mm")
    {
        ShortPaperSize = "A3";
    }
    else if (PaperSize == "na_letter_8.5x11in")
    {
        ShortPaperSize = "letter";
    }
    else if (PaperSize == "na_legal_8.5x14in")
    {
        ShortPaperSize = "legal";
    }
    else
    {
        qDebug() << "Unsupported PDF paper size" << PaperSize;
        tempfile->deleteLater();
        emit failed(tr("Unsupported PDF paper size"));
        return;
    }

    QProcess* pdftocairo = new QProcess(this);
    pdftocairo->setProgram("pdftocairo");
    pdftocairo->setArguments({"-pdf", "-paper", ShortPaperSize, filename, "-"});

    QProcess* pdftoppm = new QProcess(this);
    pdftoppm->setProgram("pdftoppm");
    QStringList Pdf2PpmArgs = {"-rx", QString::number(HwResX), "-ry", QString::number(HwResY)};
    if(Colors != 3)
    {
        Pdf2PpmArgs.append("-gray");
    }
    pdftoppm->setArguments(Pdf2PpmArgs);


    QProcess* ppm2pwg = new QProcess(this);
    // Yo dawg, I heard you like programs...
    ppm2pwg->setProgram("harbour-seaprint");
    ppm2pwg->setArguments({"ppm2pwg"});

    QStringList env;
    ppm2PwgEnv(env, urf, Quality, PaperSize, HwResX, HwResY, TwoSided, Tumble);
    qDebug() << "ppm2pwg env is " << env;

    ppm2pwg->setEnvironment(env);

    pdftocairo->setStandardOutputProcess(pdftoppm);
    pdftoppm->setStandardOutputProcess(ppm2pwg);
    ppm2pwg->setStandardOutputFile(tempfile->fileName(), QIODevice::Append);

    connect(pdftocairo, SIGNAL(finished(int, QProcess::ExitStatus)), pdftocairo, SLOT(deleteLater()));
    connect(pdftoppm, SIGNAL(finished(int, QProcess::ExitStatus)), pdftoppm, SLOT(deleteLater()));
    connect(ppm2pwg, SIGNAL(finished(int, QProcess::ExitStatus)), ppm2pwg, SLOT(deleteLater()));

    qDebug() << "All connected";

    pdftocairo->start();
    pdftoppm->start();
    ppm2pwg->start();

    qDebug() << "Starting";

    if(!pdftocairo->waitForStarted())
    {
        qDebug() << "pdftocairo died";
        tempfile->deleteLater();
        emit failed(tr("Conversion error"));
        return;
    }
    if(!pdftoppm->waitForStarted())
    {
        qDebug() << "pdftoppm died";
        tempfile->deleteLater();
        emit failed(tr("Conversion error"));
        return;
    }
    if(!ppm2pwg->waitForStarted())
    {
        qDebug() << "ppm2pwg died";
        tempfile->deleteLater();
        emit failed(tr("Conversion error"));
        return;
    }
    qDebug() << "All started";

    ppm2pwg->waitForFinished();

    qDebug() << "Finished";

    emit done(request, tempfile);
    qDebug() << "posted";
}

void ConvertWorker::convertImage(QNetworkRequest request, QString filename, QTemporaryFile* tempfile,
                                 bool urf, quint32 Colors, quint32 Quality, QString PaperSize,
                                 quint32 HwResX, quint32 HwResY)
{
    if(!PaperSizes.contains(PaperSize))
    {
        qDebug() << "Unsupported paper size" << PaperSize;
        tempfile->deleteLater();
        emit failed(tr("Unsupported paper size"));
        return;
    }
    QPair<float,float> wh = PaperSizes[PaperSize];
    quint32 Width = qRound(wh.first/25.4*HwResX);
    quint32 Height = qRound(wh.second/25.4*HwResY);

    qDebug() << "Size is" << Width << "x" << Height;

    QImage inImage;
    if(!inImage.load(filename))
    {
        qDebug() << "failed to load";
        emit failed(tr("Failed to load image"));
        return;
    }

    if(inImage.width() > inImage.height())
    {
        inImage = inImage.transformed(QMatrix().rotate(90.0));
    }
    inImage = inImage.scaled(Width, Height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QImage outImage = QImage(Width, Height, inImage.format());
    outImage.fill(Qt::white);
    QPainter painter(&outImage);
    painter.drawImage(0, (outImage.height()-inImage.height())/2, inImage);
    painter.end();

    QTemporaryFile tmpImage;
    tmpImage.open();
    qDebug() << "Raw image: " <<  tmpImage.fileName();
    outImage.save(tmpImage.fileName(), Colors == 1 ? "pgm" : "ppm");
    tmpImage.close();

    QProcess* ppm2pwg = new QProcess(this);
    // Yo dawg, I heard you like programs...
    ppm2pwg->setProgram("harbour-seaprint");
    ppm2pwg->setArguments({"ppm2pwg"});

    QStringList env;
    ppm2PwgEnv(env, urf, Quality, PaperSize, HwResX, HwResY, false, false);
    qDebug() << "ppm2pwg env is " << env;

    ppm2pwg->setEnvironment(env);
    ppm2pwg->setStandardInputFile(tmpImage.fileName());
    ppm2pwg->setStandardOutputFile(tempfile->fileName(), QIODevice::Append);

    connect(ppm2pwg, SIGNAL(finished(int, QProcess::ExitStatus)), ppm2pwg, SLOT(deleteLater()));

    qDebug() << "All connected";
    ppm2pwg->start();

    qDebug() << "Starting";

    if(!ppm2pwg->waitForStarted())
    {
        qDebug() << "ppm2pwg died";
        tempfile->deleteLater();
        emit failed(tr("Conversion error"));
        return;
    }
    qDebug() << "All started";

    ppm2pwg->waitForFinished();

    qDebug() << "Finished";

    emit done(request, tempfile);
    qDebug() << "posted";
}
