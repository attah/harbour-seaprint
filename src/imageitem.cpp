#include "imageitem.h"
#include "svgprovider.h"

ImageItem::ImageItem()
{

}

void ImageItem::paint(QPainter *painter)
{

    QImage scaled = _image.scaledToHeight(boundingRect().height(), Qt::SmoothTransformation);
    painter->drawImage(QPoint {0, 0}, scaled);
}

QImage ImageItem::getImage() const
{
    return _image;
}

void ImageItem::setImage(const QImage &image)
{
    _image = image;
    emit imageChanged();
    update();
}


bool ImageItem::isValid() const
{
    return !_image.isNull();
}
