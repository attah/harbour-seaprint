#ifndef QIPPPRINTER_H
#define QIPPPRINTER_H

#include "curlrequester.h"
#include "printparameters.h"
#include "ippprinter.h"
#include "setting.h"
#include "stringutils.h"

#include <QImage>
#include <QUrl>
#include <QJsonObject>
#include <QJsonArray>
#include <QObject>
#include <QThread>

#include <QDebug>

class Worker : public QObject
{
    Q_OBJECT
    friend class QIppPrinter;

public:
    Worker();
    ~Worker();


signals:
    void printerChanged();
    void progressChanged(size_t done, size_t total);
    void jobFinished(QString errMsg);
    void cancelJobFailed();
    void jobsFetched(QJsonArray jobs);
    void iconFetched(QImage icon);
    void stringsFetched(QJsonObject strings);

public slots:
    void cleanup();
    void urlChanged(QUrl url);
    void refresh();
    void print(QString fileName, IppPrintJob job);
    void identify();
    void getJobs();
    void cancelJob(int jobId);


protected:
    IppPrinter _printer = IppPrinter(IppAttrs {});
    std::unique_ptr<QThread> _thread;

};

class QIppIntChoiceSetting : public QObject
{
    Q_OBJECT
public:
    QIppIntChoiceSetting() = delete;

    QIppIntChoiceSetting(ChoiceSetting<int>* setting)
    : _setting(setting)
    {}

    Q_PROPERTY(int value READ get WRITE set RESET reset NOTIFY changed)
    Q_PROPERTY(QJsonArray choices READ getChoices NOTIFY changed)
    Q_PROPERTY(QString ippName READ getIppName NOTIFY changed)
    Q_PROPERTY(bool isSet READ getIsSet NOTIFY changed)

signals:
    void changed();

private:
    int get()
    {
        return _setting->get();
    }

    void set(int value)
    {
        _setting->set(value);
        emit changed();
    }

    void reset()
    {
        _setting->unset();
        emit changed();
    }

    QJsonArray getChoices()
    {
        QJsonArray res;
        for(int i : _setting->getSupported())
        {
            res.append(i);
        }
        return res;
    }

    QString getIppName()
    {
        return QString::fromStdString(_setting->name());
    }

    bool getIsSet()
    {
        return _setting->isSet();
    }

    ChoiceSetting<int>* _setting;
};

class QIppStringChoiceSetting : public QObject
{
    Q_OBJECT
public:
    QIppStringChoiceSetting() = delete;

    QIppStringChoiceSetting(ChoiceSetting<std::string>* setting)
    : _setting(setting)
    {}

    Q_PROPERTY(QString value READ get WRITE set RESET reset NOTIFY changed)
    Q_PROPERTY(QJsonArray choices READ getChoices NOTIFY changed)
    Q_PROPERTY(QString ippName READ getIppName NOTIFY changed)
    Q_PROPERTY(bool isSet READ getIsSet NOTIFY changed)

signals:
    void changed();

private:
    QString get()
    {
        return QString::fromStdString(_setting->get());
    }

    void set(QString value)
    {
        _setting->set(value.toStdString());
        emit changed();
    }

    void reset()
    {
        _setting->unset();
        emit changed();
    }

    QJsonArray getChoices()
    {
        QJsonArray res;
        for(std::string s : _setting->getSupported())
        {
            res.append(QString::fromStdString(s));
        }
        return res;
    }

    QString getIppName()
    {
        return QString::fromStdString(_setting->name());
    }

    bool getIsSet()
    {
        return _setting->isSet();
    }

    ChoiceSetting<std::string>* _setting;
};

class QIppPreferredStringChoiceSetting : public QIppStringChoiceSetting
{
    Q_OBJECT
public:
    QIppPreferredStringChoiceSetting() = delete;

    QIppPreferredStringChoiceSetting(PreferredChoiceSetting<std::string>* setting)
    : QIppStringChoiceSetting(setting), _setting(setting)
    {}

    Q_PROPERTY(QJsonArray preferredChoices READ getPreferreedChoices NOTIFY changed)

    QJsonArray getPreferreedChoices()
    {
        QJsonArray res;
        for(std::string s : _setting->getPreferred())
        {
            res.append(QString::fromStdString(s));
        }
        return res;
    }

signals:
    void changed();

private:
    PreferredChoiceSetting<std::string>* _setting;
};

class QIppResolutionChoiceSetting : public QObject
{
    Q_OBJECT
public:
    QIppResolutionChoiceSetting() = delete;

    QIppResolutionChoiceSetting(ChoiceSetting<IppResolution>* setting)
    : _setting(setting)
    {}

    Q_PROPERTY(QJsonObject value READ get WRITE set RESET reset NOTIFY changed)
    Q_PROPERTY(QJsonArray choices READ getChoices NOTIFY changed)
    Q_PROPERTY(QString ippName READ getIppName NOTIFY changed)
    Q_PROPERTY(bool isSet READ getIsSet NOTIFY changed)

signals:
    void changed();

private:
    QJsonObject get()
    {
        IppResolution res = _setting->get();
        return {{"x", (int)res.x}, {"y", (int)res.y}, {"units", (int)res.units}};
    }

    void set(QJsonObject value)
    {
        _setting->set({(uint32_t)value["x"].toInt(), (uint32_t)value["y"].toInt(), (IppResolution::Units)value["units"].toInt()});
        emit changed();
    }

    void reset()
    {
        _setting->unset();
        emit changed();
    }

    QJsonArray getChoices()
    {
        QJsonArray res;
        for(IppResolution i : _setting->getSupported())
        {
            res.append(QJsonObject {{"x", (int)i.x}, {"y", (int)i.y}, {"units", (int)i.units}});
        }
        return res;
    }

    QString getIppName()
    {
        return QString::fromStdString(_setting->name());
    }

    bool getIsSet()
    {
        return _setting->isSet();
    }

    ChoiceSetting<IppResolution>* _setting;
};

class QIppIntSetting : public QObject
{
    Q_OBJECT
public:
    QIppIntSetting() = delete;

    QIppIntSetting(IntegerSetting* setting)
    : _setting(setting)
    {}

    Q_PROPERTY(int value READ get WRITE set RESET reset NOTIFY changed)
    Q_PROPERTY(int low READ getMin NOTIFY changed)
    Q_PROPERTY(int high READ getMax NOTIFY changed)
    Q_PROPERTY(QString ippName READ getIppName NOTIFY changed)
    Q_PROPERTY(bool isSet READ getIsSet NOTIFY changed)

signals:
    void changed();

private:
    int get()
    {
        return _setting->get();
    }

    void set(int value)
    {
        _setting->set(value);
        emit changed();
    }

    void reset()
    {
        _setting->unset();
        emit changed();
    }

    int getMin()
    {
        return _setting->getSupportedMin();
    }

    int getMax()
    {
        return _setting->getSupportedMax();
    }

    QString getIppName()
    {
        return QString::fromStdString(_setting->name());
    }

    bool getIsSet()
    {
        return _setting->isSet();
    }

    IntegerSetting* _setting;
};

class QIppIntRangeListSetting : public QObject
{
    Q_OBJECT
public:
    QIppIntRangeListSetting() = delete;

    QIppIntRangeListSetting(IntegerRangeListSetting* setting)
    : _setting(setting)
    {}

    Q_PROPERTY(QJsonArray value READ get WRITE set RESET reset NOTIFY changed)
    Q_PROPERTY(QString pretty READ getPretty NOTIFY changed)
    Q_PROPERTY(QString ippName READ getIppName NOTIFY changed)
    Q_PROPERTY(bool isSet READ getIsSet NOTIFY changed)

signals:
    void changed();

private:
    QJsonArray get()
    {
        QJsonArray res;
        for(IppValue a : _setting->get())
        {
            IppIntRange ir = a.get<IppIntRange>();
            res.append(QJsonObject {{"low", ir.low}, {"high", ir.high}});
        }
        return res;
    }

    void set(QJsonArray value)
    {
        IppOneSetOf l;
        for(QJsonValue v : value)
        {
            int32_t low = v.toObject()["low"].toInt();
            int32_t high = v.toObject()["high"].toInt();
            if(high < 0)
            {
                high = std::numeric_limits<int32_t>::max();
            }
            l.push_back(IppIntRange {low, high});
        }
        _setting->set(l);
        emit changed();
    }

    QString getPretty()
    {
        List<std::string> l;
        for(IppValue a : _setting->get())
        {
            IppIntRange ir = a.get<IppIntRange>();
            l.push_back(ir.toStr());
        }
        return QString::fromStdString(join_string(l, ","));
    }

    void reset()
    {
        _setting->unset();
        emit changed();
    }

    QString getIppName()
    {
        return QString::fromStdString(_setting->name());
    }

    bool getIsSet()
    {
        return _setting->isSet();
    }

    IntegerRangeListSetting* _setting;
};

class QIppPrinter : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString url READ getUrl WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QJsonObject strings MEMBER _strings NOTIFY stringsChanged)
    Q_PROPERTY(QImage icon MEMBER _icon NOTIFY iconChanged)
    Q_PROPERTY(QString busyMessage MEMBER _busyMessage NOTIFY busyMessageChanged)
    Q_PROPERTY(QString progress MEMBER _progress NOTIFY progressChanged)

    Q_PROPERTY(QString name READ getName NOTIFY dataChanged)
    Q_PROPERTY(bool isOk READ getIsOk NOTIFY dataChanged)
    Q_PROPERTY(QStringList supportedFormats READ getSupportedFormats NOTIFY dataChanged)
    Q_PROPERTY(bool supportsPrinterRaster READ getSupportsPrinterRaster NOTIFY dataChanged)

    Q_PROPERTY(QString makeAndModel READ getMakeAndModel NOTIFY dataChanged)
    Q_PROPERTY(QString location READ getLocation NOTIFY dataChanged)
    Q_PROPERTY(int state READ getState NOTIFY dataChanged)
    Q_PROPERTY(QStringList stateReasons READ getStateReasons NOTIFY dataChanged)
    Q_PROPERTY(QString stateMessage READ getStateMessage NOTIFY dataChanged)
    Q_PROPERTY(QStringList ippVersions READ getIppVersions NOTIFY dataChanged)
    Q_PROPERTY(QStringList ippFeatures READ getIppFeatures NOTIFY dataChanged)
    Q_PROPERTY(int pagesPerMinute READ getPagesPerMinute NOTIFY dataChanged)
    Q_PROPERTY(int pagesPerMinuteColor READ getPagesPerMinuteColor NOTIFY dataChanged)
    Q_PROPERTY(QJsonArray firmware READ getFirmware NOTIFY dataChanged)
    Q_PROPERTY(QJsonArray supplies READ getSupplies NOTIFY dataChanged)
    Q_PROPERTY(bool identifySupported READ getIdentifySupported NOTIFY dataChanged)
    Q_PROPERTY(QJsonArray jobs MEMBER _jobs NOTIFY jobsChanged)


    Q_PROPERTY(bool isWarningState READ getIsWarningState NOTIFY dataChanged)


    Q_PROPERTY(QJsonObject debugJSON READ getDebugJSON NOTIFY dataChanged)

    Q_PROPERTY(bool correctSuffix READ correctSuffix NOTIFY dataChanged)
    Q_PROPERTY(QStringList suffixes READ suffixes NOTIFY dataChanged)

    Q_PROPERTY(QIppStringChoiceSetting* sides READ getSides NOTIFY dataChanged)
    Q_PROPERTY(QIppPreferredStringChoiceSetting* media READ getMedia NOTIFY dataChanged)
    Q_PROPERTY(QIppIntSetting* copies READ getCopies NOTIFY dataChanged)
    Q_PROPERTY(QIppStringChoiceSetting* multipleDocumentHandling READ getMultipleDocumentHandling NOTIFY dataChanged)

    Q_PROPERTY(QIppIntRangeListSetting* pageRanges READ getPageRanges NOTIFY dataChanged)

    Q_PROPERTY(QIppIntChoiceSetting* numberUp READ getNumberUp NOTIFY dataChanged)
    Q_PROPERTY(QIppStringChoiceSetting* colorMode READ getColorMode NOTIFY dataChanged)
    Q_PROPERTY(QIppIntChoiceSetting* printQuality READ getPrintQuality NOTIFY dataChanged)
    Q_PROPERTY(QIppResolutionChoiceSetting* resolution READ getResolution NOTIFY dataChanged)
    Q_PROPERTY(QIppStringChoiceSetting* scaling READ getScaling NOTIFY dataChanged)
    Q_PROPERTY(QIppStringChoiceSetting* documentFormat READ getDocumentFormat NOTIFY dataChanged)

    Q_PROPERTY(QIppStringChoiceSetting* mediaType READ getMediaType NOTIFY dataChanged)
    Q_PROPERTY(QIppStringChoiceSetting* mediaSource READ getMediaSource NOTIFY dataChanged)
    Q_PROPERTY(QIppStringChoiceSetting* outputBin READ getOutputBin NOTIFY dataChanged)

    Q_PROPERTY(QIppIntChoiceSetting* topMargin READ getTopMargin NOTIFY dataChanged)
    Q_PROPERTY(QIppIntChoiceSetting* bottomMargin READ getBottomMargin NOTIFY dataChanged)
    Q_PROPERTY(QIppIntChoiceSetting* leftMargin READ getLeftMargin NOTIFY dataChanged)
    Q_PROPERTY(QIppIntChoiceSetting* rightMargin READ getRightMargin NOTIFY dataChanged)


public:
    QIppPrinter();
    ~QIppPrinter();


    QString getUrl() {return _url.toString();}
    void setUrl(QString url);

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void print(QString fileName);
    Q_INVOKABLE void identify();
    Q_INVOKABLE void getJobs();
    Q_INVOKABLE void cancelJob(int jobId);


    QString getName()
    {
        return QString(_worker._printer.name().c_str());
    }

    QStringList getSupportedFormats();
    Q_INVOKABLE QStringList possibleTransferFormats(QString inputFormat);
    bool getSupportsPrinterRaster();
    QString getMakeAndModel();
    QString getLocation();
    int getState();
    QStringList getStateReasons();
    QString getStateMessage();

    QStringList getIppVersions();
    QStringList getIppFeatures();
    int getPagesPerMinute();
    int getPagesPerMinuteColor();
    QJsonArray getFirmware();
    QJsonArray getSupplies();
    bool getIdentifySupported();


    bool getIsWarningState();
    QJsonObject getDebugJSON();

    bool getIsOk();

    bool correctSuffix();
    QStringList suffixes();

signals:
    void urlChanged();
    void dataChanged();
    void jobsChanged();

    void stringsChanged();
    void iconChanged();

    void jobFinished(QString errMsg);
    void cancelJobFailed();


    void doGetStrings(QUrl url);
    void doGetImage(QUrl url);

    void busyMessageChanged();
    void progressChanged();


public slots:
    void onUrlChanged();
    void printerChanged();
    void onProgressChanged(size_t done, size_t total);
    void onJobFinished(QString errMsg);
    void onCancelJobFailed();


    void onJobsFetched(QJsonArray jobs);
    void onIconFetched(QImage icon);
    void onStringsFetched(QJsonObject strings);

private:
    QUrl _url;

    QJsonObject opAttrs();

    void setBusyMessage(QString msg);

    bool isAllowedAddress(QUrl addr);

    QJsonValue getAttrOrDefault(QJsonObject jobAttrs, QString name, QString subkey = "");


    QJsonObject _strings;
    QImage _icon;

    QStringList _additionalDocumentFormats;

    QString _busyMessage;
    QString _progress;
    QJsonArray _jobs;

    Worker _worker;
    IppPrintJob _job;

    QIppStringChoiceSetting _sides;
    QIppPreferredStringChoiceSetting _media;
    QIppIntSetting _copies;
    QIppStringChoiceSetting _multipleDocumentHandling;

    QIppIntRangeListSetting _pageRanges;

    QIppIntChoiceSetting _numberUp;
    QIppStringChoiceSetting _colorMode;
    QIppIntChoiceSetting _printQuality;
    QIppResolutionChoiceSetting _resolution;

    QIppStringChoiceSetting _scaling;
    QIppStringChoiceSetting _documentFormat;

    QIppStringChoiceSetting _mediaType;
    QIppStringChoiceSetting _mediaSource;
    QIppStringChoiceSetting _outputBin;

    QIppIntChoiceSetting _topMargin;
    QIppIntChoiceSetting _bottomMargin;
    QIppIntChoiceSetting _leftMargin;
    QIppIntChoiceSetting _rightMargin;

    QIppStringChoiceSetting* getSides()
    {
        return &_sides;
    }

    QIppPreferredStringChoiceSetting* getMedia()
    {
        return &_media;
    }

    QIppIntSetting* getCopies()
    {
        return &_copies;
    }

    QIppStringChoiceSetting* getMultipleDocumentHandling()
    {
        return &_multipleDocumentHandling;
    }

    QIppIntRangeListSetting* getPageRanges()
    {
        return &_pageRanges;
    }

    QIppIntChoiceSetting* getNumberUp()
    {
        return &_numberUp;
    }

    QIppStringChoiceSetting* getColorMode()
    {
        return &_colorMode;
    }

    QIppIntChoiceSetting* getPrintQuality()
    {
        return &_printQuality;
    }

    QIppResolutionChoiceSetting* getResolution()
    {
        return &_resolution;
    }

    QIppStringChoiceSetting* getScaling()
    {
        return &_scaling;
    }

    QIppStringChoiceSetting* getDocumentFormat()
    {
        return &_documentFormat;
    }

    QIppStringChoiceSetting* getMediaType()
    {
        return &_mediaType;
    }

    QIppStringChoiceSetting* getMediaSource()
    {
        return &_mediaSource;
    }

    QIppStringChoiceSetting* getOutputBin()
    {
        return &_outputBin;
    }

    QIppIntChoiceSetting* getTopMargin()
    {
        return &_topMargin;
    }

    QIppIntChoiceSetting* getBottomMargin()
    {
        return &_bottomMargin;
    }

    QIppIntChoiceSetting* getLeftMargin()
    {
        return &_leftMargin;
    }

    QIppIntChoiceSetting* getRightMargin()
    {
        return &_rightMargin;
    }


};

#endif // QIPPPRINTER_H
