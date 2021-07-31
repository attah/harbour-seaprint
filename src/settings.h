#ifndef SETTINGS_H
#define SETTINGS_H

#include <mlite5/MGConfItem>

class Settings : public QObject
   {
       Q_OBJECT
public:

    Q_PROPERTY(QString ignoreSslErrorsPath READ ignoreSslErrorsPath CONSTANT)
    Q_PROPERTY(bool ignoreSslErrorsDefault MEMBER _ignoreSslErrorsDefault CONSTANT)

public:
    static Settings* instance();

    bool ignoreSslErrors();

private:
    Settings();
    ~Settings();
    Settings(const Settings &);
    Settings& operator=(const Settings &);

    static Settings* m_Instance;

    MGConfItem _ignoreSslErrorsSetting;

    QString ignoreSslErrorsPath();

    bool _ignoreSslErrorsDefault = true;
};

#endif // SETTINGS_H
