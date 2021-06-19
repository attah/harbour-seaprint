#ifndef SETTINGS_H
#define SETTINGS_H

#include <mlite5/MGConfItem>

class Settings : public QObject
   {
       Q_OBJECT
public:
    Q_PROPERTY(QString alwaysUseMediaColPath READ alwaysUseMediaColPath CONSTANT)
    Q_PROPERTY(bool alwaysUseMediaColDefault MEMBER _alwaysUseMediaColDefault CONSTANT)

    Q_PROPERTY(QString ignoreSslErrorsPath READ ignoreSslErrorsPath CONSTANT)
    Q_PROPERTY(bool ignoreSslErrorsDefault MEMBER _ignoreSslErrorsDefault CONSTANT)

public:
    static Settings* instance();

    bool alwaysUseMediaCol();
    bool ignoreSslErrors();

private:
    Settings();
    ~Settings();
    Settings(const Settings &);
    Settings& operator=(const Settings &);

    static Settings* m_Instance;

    MGConfItem _alwaysUseMediaColSetting;
    MGConfItem _ignoreSslErrorsSetting;

    QString alwaysUseMediaColPath();
    QString ignoreSslErrorsPath();

    bool _alwaysUseMediaColDefault = false;
    bool _ignoreSslErrorsDefault = true;
};

#endif // SETTINGS_H
