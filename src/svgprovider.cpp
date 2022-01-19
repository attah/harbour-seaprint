#include "svgprovider.h"


SvgProvider::SvgProvider() : QQuickImageProvider(QQuickImageProvider::Image, ForceAsynchronousImageLoading)
{

}

SvgProvider::~SvgProvider() {
}

SvgProvider* SvgProvider::m_Instance = nullptr;

SvgProvider* SvgProvider::instance()
{
    static QMutex mutex;
    if (!m_Instance)
    {
        mutex.lock();

        if (!m_Instance)
            m_Instance = new SvgProvider;

        mutex.unlock();
    }

    return m_Instance;
}

QImage SvgProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QSvgRenderer renderer(SailfishApp::pathTo(id).toLocalFile());
    *size = requestedSize.isValid() ? requestedSize : renderer.defaultSize();

    QImage img(*size, QImage::Format_ARGB32);
    img.fill(QColor("transparent"));
    QPainter painter(&img);

    renderer.render(&painter);

    return img;
}
