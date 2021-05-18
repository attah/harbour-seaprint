#ifndef SVGPROVIDER_H
#define SVGPROVIDER_H

#include <QQuickImageProvider>
#include <QPainter>
#include <QtSvg/QSvgRenderer>
#include <QMutex>
#include <QtDebug>
#include <sailfishapp.h>

class SvgProvider : public QQuickImageProvider
{
public:
    static SvgProvider* instance();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    SvgProvider();
    ~SvgProvider();
    SvgProvider(const SvgProvider &);
    SvgProvider& operator=(const SvgProvider &);

    static SvgProvider* m_Instance;

};

#endif // SVGPROVIDER_H
