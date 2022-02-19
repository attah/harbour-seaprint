#ifndef IMAGEITEM_H
#define IMAGEITEM_H

#include <QQuickPaintedItem>
#include <QPainter>
#include <QImage>

class ImageItem : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(QImage image READ getImage WRITE setImage NOTIFY imageChanged)
    Q_PROPERTY(bool valid READ isValid NOTIFY imageChanged)
public:
    ImageItem();

    void paint(QPainter *painter);

signals:
    void imageChanged();

private:
    QImage getImage() const;
    void setImage(const QImage &image);

    bool isValid() const;

    QImage _image;
};

#endif // IMAGEITEM_H
