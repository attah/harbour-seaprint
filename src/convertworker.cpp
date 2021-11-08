#include "convertworker.h"
#include <sailfishapp.h>
#include "papersizes.h"
#include "convertchecker.h"
#include "mimer.h"
#include "seaprint_version.h"
#include <QImage>
#include <QMatrix>
#include <QPainter>
#include <QTextDocument>
#include <QPdfWriter>
#include <QAbstractTextDocumentLayout>
#include "ippprinter.h"
#include "pdf2printable.h"

void ppm2PwgEnv(QStringList& env, bool urf, quint32 Quality, QString PaperSize,
                quint32 HwResX, quint32 HwResY, bool TwoSided, bool Tumble,
                bool ForcePortrait, quint32 pages, bool BackHFlip, bool BackVFlip)
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

    if(ForcePortrait)
    {
        env.append("FORCE_PORTRAIT=true");
    }

    if(pages != 0)
    {
        env.append("PAGES="+QString::number(pages));
    }

    if(BackHFlip)
    {
        env.append("BACK_HFLIP=true");
    }
    if(BackVFlip)
    {
        env.append("BACK_VFLIP=true");
    }
}

ConvertWorker::ConvertWorker(IppPrinter* parent) // : QObject((QObject*)parent) borks multithereading?!
{
    _printer = parent;
}

void ConvertWorker::command(QByteArray msg)
{
    CurlRequester cid(_printer->httpUrl());
    cid.setFinishedCallback(_printer, &IppPrinter::getPrinterAttributesFinished);

    qDebug() << "command...";

    cid.write(msg.data(), msg.length());
}

// TODO: de-duplicate
void ConvertWorker::getJobs(QByteArray msg)
{
    CurlRequester cid(_printer->httpUrl());
    cid.setFinishedCallback(_printer, &IppPrinter::getJobsRequestFinished);

    cid.write(msg.data(), msg.length());
}

void ConvertWorker::cancelJob(QByteArray msg)
{
    CurlRequester cid(_printer->httpUrl());
    cid.setFinishedCallback(_printer, &IppPrinter::cancelJobFinished);

    cid.write(msg.data(), msg.length());
}

void ConvertWorker::justUpload(QString filename, QByteArray header)
{
    qDebug() << "justupload";

    CurlRequester cid(_printer->httpUrl());
    cid.setFinishedCallback(_printer, &IppPrinter::printRequestFinished);

    QFile file(filename);
    file.open(QFile::ReadOnly);

    cid.write(header.data(), header.length());
    QByteArray tmp = file.readAll();
    cid.write(tmp.data(), tmp.length());
    file.close();
}

void ConvertWorker::convertPdf(QString filename, QByteArray header,
                               QString targetFormat, quint32 Colors, quint32 Quality, QString PaperSize,
                               quint32 HwResX, quint32 HwResY, bool TwoSided, bool Tumble,
                               quint32 PageRangeLow, quint32 PageRangeHigh, bool BackHFlip, bool BackVFlip)
{
try {
    Format format;

    qDebug() << "to pdf" << HwResX << HwResY;

    if(targetFormat == Mimer::URF)
    {
        format = Format::URF;
    }
    else if(targetFormat == Mimer::PWG)
    {
        format = Format::PWG;
    }
    else if(targetFormat == Mimer::Postscript)
    {
        format = Format::Postscript;
    }
    else if (targetFormat == Mimer::PDF)
    {
        format = Format::PDF;
    }
    else
    {
        throw ConvertFailedException(tr("Unsupported target format"));
    }

    if(Colors == 0)
    {
        Colors = 3;
    }

    CurlRequester cid(_printer->httpUrl());
    cid.setFinishedCallback(_printer, &IppPrinter::printRequestFinished);

    cid.write(header.data(), header.length());

    write_fun WriteFun([&cid](unsigned char const* buf, unsigned int len) -> bool
              {
                qDebug() << "wf called " << len;
                cid.write((const char*)buf, len);
                qDebug() << "wf returns " << len;
                return true;
              });

    if(!PaperSizes.contains(PaperSize))
    {
        qDebug() << "Unsupported paper size" << PaperSize;
        throw ConvertFailedException(tr("Unsupported paper size"));
    }
    QSizeF size = PaperSizes[PaperSize];
    float Width = size.width();
    float Height = size.height();

    int res = pdf_to_printable(filename.toStdString(), WriteFun, Colors, Quality, PaperSize.toStdString(), Width, Height, HwResX, HwResY,
                               format, TwoSided, Tumble, BackHFlip, BackVFlip, PageRangeLow, PageRangeHigh);

    if(res != 0)
    {
        throw ConvertFailedException("Conversion failed");
    }

    qDebug() << "Finished";
}
catch(const ConvertFailedException& e)
{
        emit failed(e.what() == QString("") ? tr("Conversion error") : e.what());
}
}

void ConvertWorker::convertImage(QString filename, QByteArray header,
                                 QString targetFormat, quint32 Colors, quint32 Quality, QString PaperSize,
                                 quint32 HwResX, quint32 HwResY, QMargins margins)
{
try {

    bool urf = false;
    bool pdfOrPostscript = false;
    QString imageFormat = "";
    QStringList supportedImageFormats = {Mimer::JPEG, Mimer::PNG};

    if(targetFormat == Mimer::URF)
    {
        urf = true;
    }
    else if(targetFormat == Mimer::PWG)
    {
        //ok
    }
    else if(targetFormat == Mimer::PDF || targetFormat == Mimer::Postscript)
    {
        HwResX = HwResY = std::min(HwResX, HwResY);
        pdfOrPostscript = true;
    }
    else if(supportedImageFormats.contains(targetFormat))
    {
        imageFormat = targetFormat.split("/")[1];
    }
    else
    {
        throw ConvertFailedException(tr("Unsupported target format"));
    }

    if(urf && (HwResX != HwResY))
    { // URF only supports symmetric resolutions
        qDebug() << "Unsupported URF resolution" << PaperSize;
        throw ConvertFailedException(tr("Unsupported resolution (dpi)"));
    }

    if(!PaperSizes.contains(PaperSize))
    {
        qDebug() << "Unsupported paper size" << PaperSize;
        throw ConvertFailedException(tr("Unsupported paper size"));
    }
    QSizeF size = PaperSizes[PaperSize];
    quint32 Width = qRound(size.width()/25.4*HwResX);
    quint32 Height = qRound(size.height()/25.4*HwResY);

    qDebug() << "Size is" << Width << "x" << Height;

    QImage inImage;
    if(!inImage.load(filename))
    {
        qDebug() << "failed to load";
        throw ConvertFailedException(tr("Failed to load image"));
    }

    if(inImage.width() > inImage.height())
    {
        inImage = inImage.transformed(QMatrix().rotate(90.0));
    }

    int leftMarginPx = (margins.left()/2540.0)*HwResX;
    int rightMarginPx = (margins.right()/2540.0)*HwResX;
    int topMarginPx = (margins.top()/2540.0)*HwResY;
    int bottomMarginPx = (margins.bottom()/2540.0)*HwResY;

    int totalXMarginPx = leftMarginPx+rightMarginPx;
    int totalYMarginPx = topMarginPx+bottomMarginPx;

    inImage = inImage.scaled(Width-totalXMarginPx, Height-totalYMarginPx,
                             Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QTemporaryFile tempfile;
    tempfile.open();
    tempfile.close();

    if(pdfOrPostscript)
    {
        QTemporaryFile tmpPdfFile;
        tmpPdfFile.open();
        QPdfWriter pdfWriter(tmpPdfFile.fileName());
        pdfWriter.setCreator("SeaPrint " SEAPRINT_VERSION);
        QPageSize pageSize(size, QPageSize::Millimeter);
        pdfWriter.setPageSize(pageSize);
        pdfWriter.setResolution(HwResX);
        QPainter painter(&pdfWriter);
        int xOffset = ((pdfWriter.width()-totalXMarginPx)-inImage.width())/2 + leftMarginPx;
        int yOffset = ((pdfWriter.height()-totalYMarginPx)-inImage.height())/2 + topMarginPx;
        painter.drawImage(xOffset, yOffset, inImage);
        painter.end();

        if(targetFormat == Mimer::PDF)
        {
            QFile tempfileAsFile(tempfile.fileName());
            tempfileAsFile.open(QIODevice::Append);
            tempfileAsFile.write(tmpPdfFile.readAll());
            tempfileAsFile.close();
        }
        else if(targetFormat == Mimer::Postscript)
        {
            pdftoPs(PaperSize, false, 0, 0, tmpPdfFile.fileName(), &tempfile);
        }

    }
    else
    {
        QImage outImage = QImage(Width, Height, inImage.format());
        outImage.fill(Qt::white);
        QPainter painter(&outImage);
        int xOffset = ((outImage.width()-totalXMarginPx)-inImage.width())/2 + leftMarginPx;
        int yOffset = ((outImage.height()-totalYMarginPx)-inImage.height())/2 + topMarginPx;
        painter.drawImage(xOffset, yOffset, inImage);
        painter.end();

        if(imageFormat != "")
        { // We are converting to a supported image format

            QFile tempfileAsFile(tempfile.fileName());
            tempfileAsFile.open(QIODevice::Append);
            outImage.save(&tempfileAsFile, imageFormat.toStdString().c_str());
            tempfileAsFile.close();
        }
        else
        { // We are converting to a raster format
            QProcess ppm2pwg(this);
            // Yo dawg, I heard you like programs...
            ppm2pwg.setProgram("harbour-seaprint");
            ppm2pwg.setArguments({"ppm2pwg"});

            QStringList env;
            ppm2PwgEnv(env, urf, Quality, PaperSize, HwResX, HwResY, false, false, false, 0, false, false);
            qDebug() << "ppm2pwg env is " << env;

            ppm2pwg.setEnvironment(env);
            ppm2pwg.setStandardOutputFile(tempfile.fileName(), QIODevice::WriteOnly);

            qDebug() << "All connected";
            ppm2pwg.start();

            bool gray = Colors == 0 ? inImage.allGray() : Colors == 1;

            outImage.save(&ppm2pwg, gray ? "pgm" : "ppm");

            qDebug() << "Starting";

            if(!ppm2pwg.waitForStarted())
            {
                qDebug() << "ppm2pwg died";
                throw ConvertFailedException();
            }
            qDebug() << "All started";

            ppm2pwg.waitForFinished();

            qDebug() << "Finished";
        }
    }

    justUpload(tempfile.fileName(), header);
    qDebug() << "posted";

}
catch(const ConvertFailedException& e)
{
    emit failed(e.what() == QString("") ? tr("Conversion error") : e.what());
}
}

void ConvertWorker::convertOfficeDocument(QString filename, QByteArray header,
                                          QString targetFormat, quint32 Colors, quint32 Quality, QString PaperSize,
                                          quint32 HwResX, quint32 HwResY, bool TwoSided, bool Tumble,
                                          quint32 PageRangeLow, quint32 PageRangeHigh, bool BackHFlip, bool BackVFlip)
{
try {

    if(targetFormat == Mimer::URF && (HwResX != HwResY))
    { // URF only supports symmetric resolutions
        qDebug() << "Unsupported URF resolution" << PaperSize;
        throw ConvertFailedException(tr("Unsupported resolution (dpi)"));
    }

    QString ShortPaperSize;
    if(CalligraPaperSizes.contains(PaperSize))
    {
        ShortPaperSize = CalligraPaperSizes[PaperSize];
    }
    else
    {
        qDebug() << "Unsupported PDF paper size" << PaperSize;
        throw ConvertFailedException(tr("Unsupported PDF paper size"));
    }

    QProcess CalligraConverter(this);
    CalligraConverter.setProgram("calligraconverter");
    QStringList CalligraConverterArgs = {"--batch", "--mimetype", Mimer::PDF, "--print-orientation", "Portrait", "--print-papersize", ShortPaperSize};

    CalligraConverterArgs << filename;

    QTemporaryFile tmpPdfFile;
    tmpPdfFile.open();
    CalligraConverterArgs << tmpPdfFile.fileName();

    qDebug() << "CalligraConverteArgs is" << CalligraConverterArgs;
    CalligraConverter.setArguments(CalligraConverterArgs);

    CalligraConverter.start();

    qDebug() << "CalligraConverter Starting";

    if(!CalligraConverter.waitForStarted())
    {
        qDebug() << "CalligraConverter died";
        throw ConvertFailedException();
    }

    qDebug() << "CalligraConverter Started";

    if(!CalligraConverter.waitForFinished(-1))
    {
        qDebug() << "CalligraConverter failed";
        throw ConvertFailedException();
    }

//    qDebug() << CalligraConverter->readAllStandardError();

    quint32 pages = ConvertChecker::instance()->pdfPages(tmpPdfFile.fileName());
    if (!pages)
    {
        qDebug() << "pdfinfo returned 0 pages";
        throw ConvertFailedException(tr("Failed to get info about PDF file"));
    }

    if(PageRangeLow==0)
    {
        PageRangeLow=1;
    }

    if(PageRangeHigh==0 || PageRangeHigh > pages)
    {
        PageRangeHigh=pages;
    }

    // Actual number of pages to print
    pages = PageRangeHigh-PageRangeLow+1;

    qDebug() << "PageRangeLow" << PageRangeLow << "PageRangeHigh" << PageRangeHigh << "pages" << pages;

    QTemporaryFile tempfile;
    tempfile.open();
    tempfile.close();

    if(targetFormat == Mimer::PDF)
    {

        if(PageRangeLow != 1 || PageRangeHigh != pages)
        {
            qDebug() << "adjusting pages in PDF" << PageRangeLow << PageRangeHigh;

            adjustPageRange(PaperSize, PageRangeLow, PageRangeHigh, tmpPdfFile.fileName(), &tempfile);

        }
        else
        {
            QFile tempfileAsFile(tempfile.fileName());
            tempfileAsFile.open(QIODevice::Append);
            tempfileAsFile.write(tmpPdfFile.readAll());
            tempfileAsFile.close();
        }

    }
    else if(targetFormat == Mimer::Postscript)
    {
        pdftoPs(PaperSize, TwoSided, PageRangeLow, PageRangeHigh, tmpPdfFile.fileName(), &tempfile);
    }
    else
    {

        pdfToRaster(targetFormat, Colors, Quality, PaperSize,
                    HwResX, HwResY, TwoSided, Tumble,
                    PageRangeLow, PageRangeHigh, pages, BackHFlip, BackVFlip,
                    tmpPdfFile.fileName(), &tempfile, false);
    }

    qDebug() << "Finished";

    justUpload(tempfile.fileName(), header);
    qDebug() << "posted";

}
catch(const ConvertFailedException& e)
{
        emit failed(e.what() == QString("") ? tr("Conversion error") : e.what());
}
}

void ConvertWorker::convertPlaintext(QString filename, QByteArray header,
                                     QString targetFormat, quint32 Colors, quint32 Quality, QString PaperSize,
                                     quint32 HwResX, quint32 HwResY, bool TwoSided, bool Tumble,
                                     bool BackHFlip, bool BackVFlip)
{
try {

    if(!PaperSizes.contains(PaperSize))
    {
        qDebug() << "Unsupported paper size" << PaperSize;
        throw ConvertFailedException(tr("Unsupported paper size"));
    }
    QSizeF size = PaperSizes[PaperSize];

    QFile inFile(filename);
    if(!inFile.open(QIODevice::ReadOnly))
    {
        throw ConvertFailedException(tr("Failed to open file"));
    }

    quint32 resolution = std::min(HwResX, HwResY);

    QTemporaryFile tmpPdfFile;
    tmpPdfFile.open();

    QPdfWriter pdfWriter(tmpPdfFile.fileName());
    pdfWriter.setCreator("SeaPrint " SEAPRINT_VERSION);
    QPageSize pageSize(size, QPageSize::Millimeter);
    pdfWriter.setPageSize(pageSize);
    pdfWriter.setResolution(resolution);

    qreal docHeight = pageSize.sizePixels(resolution).height();

    QTextDocument doc;

    QFont font = QFont("Courier");
    font.setPointSizeF(1);

    qreal charHeight = 0;

    // Find the optimal font size
    while(true) {
        QFont tmpFont = font;
        tmpFont.setPointSizeF(font.pointSizeF()+0.5);
        QFontMetricsF qfm(tmpFont, &pdfWriter);

        charHeight = qfm.lineSpacing();

        if(charHeight*66 > docHeight)
        {
            break;
        }
        font=tmpFont;
    }

    QFontMetricsF qfm(font, &pdfWriter);

    charHeight = qfm.height();

    int textHeight = 60*charHeight;
    qreal margin = ((docHeight-textHeight)/2);
    qreal mmMargin = margin/(resolution/25.4);

    doc.setDefaultFont(font);
    (void)doc.documentLayout(); // wat


    // Needs to be before painter
    pdfWriter.setMargins({mmMargin, mmMargin, mmMargin, mmMargin});

    QPainter painter(&pdfWriter);

    doc.documentLayout()->setPaintDevice(painter.device());
    doc.setDocumentMargin(margin);

    // Hack to make the document and pdfWriter margins overlap
    // Apparently calls to painter.translate() stack... who knew!
    painter.translate(-margin, -margin);

    QRectF body = pageSize.rectPixels(resolution);
    doc.setPageSize(body.size());

    QString allText = inFile.readAll();
    if(allText.startsWith("\f"))
    {
        allText.remove(0, 1);
    }

    if(allText.endsWith("\f"))
    {
        allText.chop(1);
    }
    else if(allText.endsWith("\f\n"))
    {
        allText.chop(2);
    }

    QStringList pages = allText.split('\f');

    bool first = true;
    int pageCount = 0;

    foreach(QString page, pages)
    {
        if(!first)
        {
            pdfWriter.newPage();
        }
        first = false;

        if(page.endsWith("\n"))
        {
            page.chop(1);
        }
        doc.setPlainText(page);

        int p = 0; // Page number in this document, starting from 0

        while(true)
        {
            painter.translate(body.left(), body.top() - p*body.height());
            QRectF view(0, p*body.height(), body.width(), body.height());
            painter.setClipRect(view);

            QAbstractTextDocumentLayout::PaintContext context;
            context.clip = view;
            context.palette.setColor(QPalette::Text, Qt::black);
            doc.documentLayout()->draw(&painter, context);

            p++;
            pageCount++;

            if(p >= doc.pageCount())
                break;

            pdfWriter.newPage();
        }
    }

    painter.end();

    QTemporaryFile tempfile;
    tempfile.open();
    tempfile.close();

    if(targetFormat == Mimer::PDF)
    {
        QFile tempfileAsFile(tempfile.fileName());
        tempfileAsFile.open(QIODevice::Append);
        tempfileAsFile.write(tmpPdfFile.readAll());
        tempfileAsFile.close();
    }
    else if(targetFormat == Mimer::Postscript)
    {
        pdftoPs(PaperSize, TwoSided, 0, 0, tmpPdfFile.fileName(), &tempfile);
    }
    else
    {
        pdfToRaster(targetFormat, Colors, Quality, PaperSize,
                    HwResX, HwResY, TwoSided, Tumble,
                    0, 0, pageCount, BackHFlip, BackVFlip,
                    tmpPdfFile.fileName(), &tempfile, false);
    }

    qDebug() << "Finished";

    justUpload(tempfile.fileName(), header);
    qDebug() << "posted";

}
catch(const ConvertFailedException& e)
{
        emit failed(e.what() == QString("") ? tr("Conversion error") : e.what());
}
}

QString ConvertWorker::getPopplerShortPaperSize(QString PaperSize)
{
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
        throw ConvertFailedException(tr("Unsupported PDF paper size"));
    }
    return ShortPaperSize;
}

void ConvertWorker::adjustPageRange(QString PaperSize, quint32 PageRangeLow, quint32 PageRangeHigh,
                                    QString pdfFileName, QTemporaryFile* tempfile)
{
    QProcess pdftocairo(this);
    pdftocairo.setProgram("pdftocairo");
    QStringList PdfToCairoArgs = {"-pdf"};

    QString ShortPaperSize = getPopplerShortPaperSize(PaperSize);

    PdfToCairoArgs << QStringList {"-f", QString::number(PageRangeLow), "-l", QString::number(PageRangeHigh)};

    PdfToCairoArgs << QStringList {"-paper", ShortPaperSize, pdfFileName, "-"};

    qDebug() << "pdftocairo args is " << PdfToCairoArgs;
    pdftocairo.setArguments(PdfToCairoArgs);

    pdftocairo.setStandardOutputFile(tempfile->fileName(), QIODevice::WriteOnly);

    pdftocairo.start();

    qDebug() << "Starting";

    if(!pdftocairo.waitForStarted())
    {
        qDebug() << "pdftocairo died";
        throw ConvertFailedException();
    }

    qDebug() << "Started";

    if(!pdftocairo.waitForFinished(-1))
    {
        qDebug() << "pdftocairo failed";
        throw ConvertFailedException();
    }
}

void ConvertWorker::pdftoPs(QString PaperSize, bool TwoSided, quint32 PageRangeLow, quint32 PageRangeHigh,
                            QString pdfFileName, QTemporaryFile* tempfile)
{
    QProcess pdftops(this);
    pdftops.setProgram("pdftops");
    QStringList PdfToPsArgs;
    if(TwoSided)
    {
        PdfToPsArgs.append("-duplex");
    }

    QString ShortPaperSize = getPopplerShortPaperSize(PaperSize);

    if(PageRangeLow != 0 && PageRangeHigh != 0)
    {
        PdfToPsArgs << QStringList {"-f", QString::number(PageRangeLow), "-l", QString::number(PageRangeHigh)};
    }

    PdfToPsArgs << QStringList {"-paper", ShortPaperSize, pdfFileName, "-"};

    qDebug() << "pdftops args is " << PdfToPsArgs;
    pdftops.setArguments(PdfToPsArgs);

    pdftops.setStandardOutputFile(tempfile->fileName(), QIODevice::Append);

    pdftops.start();

    qDebug() << "Starting";

    if(!pdftops.waitForStarted())
    {
        qDebug() << "pdftops died";
        throw ConvertFailedException();
    }

    qDebug() << "Started";

    if(!pdftops.waitForFinished(-1))
    {
        qDebug() << "pdftops failed";
        throw ConvertFailedException();
    }
}

void ConvertWorker::pdfToRaster(QString targetFormat, quint32 Colors, quint32 Quality, QString PaperSize,
                                quint32 HwResX, quint32 HwResY,  bool TwoSided, bool Tumble,
                                quint32 PageRangeLow, quint32 PageRangeHigh, quint32 pages,
                                bool BackHFlip, bool BackVFlip,
                                QString pdfFileName, QTemporaryFile* tempfile, bool resize)
{

    if(PageRangeLow==0)
    {
        PageRangeLow=1;
    }

    if(PageRangeHigh==0)
    {
        PageRangeHigh=pages;
    }

    // Actual number of pages to print
    pages = PageRangeHigh-PageRangeLow+1;
    qDebug() << "PageRangeLow" << PageRangeLow << "PageRangeHigh" << PageRangeHigh << "pages" << pages;

    QProcess pdftocairo(this);
    pdftocairo.setProgram("pdftocairo");
    QStringList PdfToCairoArgs;

    QProcess pdftoppm(this);
    pdftoppm.setProgram("pdftoppm");
    QStringList Pdf2PpmArgs = {"-rx", QString::number(HwResX), "-ry", QString::number(HwResY)};

    if(resize)
    {
        QString ShortPaperSize = getPopplerShortPaperSize(PaperSize);

        PdfToCairoArgs << QStringList {"-f", QString::number(PageRangeLow), "-l", QString::number(PageRangeHigh)};
        PageRangeLow = PageRangeHigh = 0;
        PdfToCairoArgs << QStringList {"-pdf", "-paper", ShortPaperSize, pdfFileName, "-"};

        pdftocairo.setArguments(PdfToCairoArgs);

        pdftocairo.setStandardOutputProcess(&pdftoppm);
    }
    else
    {
        Pdf2PpmArgs << QStringList {"-f", QString::number(PageRangeLow), "-l", QString::number(PageRangeHigh)};
        pdftoppm.setStandardInputFile(pdfFileName);

    }

    if(Colors == 1)
    {
        Pdf2PpmArgs.append("-gray");
    }
    qDebug() << "pdf2ppm args is " << Pdf2PpmArgs;
    pdftoppm.setArguments(Pdf2PpmArgs);


    QProcess ppm2pwg(this);
    // Yo dawg, I heard you like programs...
    ppm2pwg.setProgram("harbour-seaprint");
    ppm2pwg.setArguments({"ppm2pwg"});

    bool urf = targetFormat == Mimer::URF;

    QStringList env;
    ppm2PwgEnv(env, urf, Quality, PaperSize, HwResX, HwResY, TwoSided, Tumble, true, pages, BackHFlip, BackVFlip);
    qDebug() << "ppm2pwg env is " << env;

    ppm2pwg.setEnvironment(env);

    pdftoppm.setStandardOutputProcess(&ppm2pwg);
    ppm2pwg.setStandardOutputFile(tempfile->fileName(), QIODevice::WriteOnly);

    qDebug() << "All connected";

    if(resize)
    {
        pdftocairo.start();
    }

    pdftoppm.start();
    ppm2pwg.start();

    qDebug() << "Starting";

    if(!pdftoppm.waitForStarted())
    {
        qDebug() << "pdftoppm died";
        throw ConvertFailedException();
    }
    if(!ppm2pwg.waitForStarted())
    {
        qDebug() << "ppm2pwg died";
        throw ConvertFailedException();
    }
    qDebug() << "All started";

    bool ppm2pwgSuccess = false;

    for(;;)
    {
        if(ppm2pwg.waitForFinished(250))
        {
            ppm2pwgSuccess = true;
            break;
        }
        else
        {
            QList<QByteArray> ppm2pwgOutput = ppm2pwg.readAllStandardError().split('\n');
            for(QList<QByteArray>::iterator it = ppm2pwgOutput.begin(); it != ppm2pwgOutput.end(); it++)
            {
                if(it->startsWith("Page"))
                {
                    QList<QByteArray> ppm2pwgTokens = it->split(' ');
                    emit progress(ppm2pwgTokens.last().toInt()-1, pages);
                }
            }
        }
    }
    if(!ppm2pwgSuccess)
    {
        qDebug() << "ppm2pwg failed";
        throw ConvertFailedException();
    }
}
