#ifndef SETTINGS_H
#define SETTINGS_H

#include <mlite5/MGConfItem>

class Settings : public QObject
   {
       Q_OBJECT
public:

    Q_PROPERTY(QString ignoreSslErrorsPath READ ignoreSslErrorsPath CONSTANT)
    Q_PROPERTY(bool ignoreSslErrorsDefault MEMBER _ignoreSslErrorsDefault CONSTANT)
    Q_PROPERTY(QString debugLogPath READ debugLogPath CONSTANT)
    Q_PROPERTY(bool debugLogDefault MEMBER _debugLogDefault CONSTANT)

public:
    static Settings* instance();

    bool ignoreSslErrors();
    bool debugLog();

private:
    Settings();
    ~Settings();
    Settings(const Settings &);
    Settings& operator=(const Settings &);

    static Settings* m_Instance;

    MGConfItem _ignoreSslErrorsSetting;
    MGConfItem _debugLogSetting;

    QString ignoreSslErrorsPath();
    QString debugLogPath();

    bool _ignoreSslErrorsDefault = true;
    bool _debugLogDefault = false;

};

#endif // SETTINGS_H
