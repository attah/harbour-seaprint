#include "settings.h"
#include <QMutex>


Settings::Settings() : _alwaysUseMediaColSetting("/apps/harbour-seaprint/settings/always-use-media-col", this),
                       _ignoreSslErrorsSetting("/apps/harbour-seaprint/settings/ignore-ssl-errors", this)
{

}

Settings::~Settings() {
}

Settings* Settings::m_Instance = nullptr;

Settings* Settings::instance()
{
    static QMutex mutex;
    if (!m_Instance)
    {
        mutex.lock();

        if (!m_Instance)
            m_Instance = new Settings;

        mutex.unlock();
    }

    return m_Instance;
}


bool Settings::alwaysUseMediaCol()
{
    return _alwaysUseMediaColSetting.value(_alwaysUseMediaColDefault).toBool();
}

bool Settings::ignoreSslErrors()
{
    return _ignoreSslErrorsSetting.value(_ignoreSslErrorsDefault).toBool();
}


QString Settings::alwaysUseMediaColPath()
{
    return _alwaysUseMediaColSetting.key();
}

QString Settings::ignoreSslErrorsPath()
{
    return _ignoreSslErrorsSetting.key();
}
