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

    Q_PROPERTY(QString allowExternalConnectionsPath READ allowExternalConnectionsPath CONSTANT)
    Q_PROPERTY(bool allowExternalConnectionsDefault MEMBER _allowExternalConnectionsDefault CONSTANT)

public:
    static Settings* instance();

    bool ignoreSslErrors();
    bool debugLog();
    bool allowExternalConnections();

private:
    Settings();
    ~Settings();
    Settings(const Settings &);
    Settings& operator=(const Settings &);

    static Settings* m_Instance;

    MGConfItem _ignoreSslErrorsSetting;
    MGConfItem _debugLogSetting;
    MGConfItem _allowExternalConnectionsSetting;

    QString ignoreSslErrorsPath();
    QString debugLogPath();
    QString allowExternalConnectionsPath();

    bool _ignoreSslErrorsDefault = true;
    bool _debugLogDefault = false;
    bool _allowExternalConnectionsDefault = false;

};

#endif // SETTINGS_H
