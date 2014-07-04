#include "QPixmapToolTip.h"

using namespace teramanager;

QPixmapToolTip* QPixmapToolTip::uniqueInstance = 0;

QPixmapToolTip::QPixmapToolTip(QWidget *parent) : QWidget(parent)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    // layout
//    QVBoxLayout* layout = new QVBoxLayout();
//    layout->addWidget(toolBar);
//    layout->setContentsMargins(0,0,0,0);
//    setLayout(layout);


    // make it appear as a true toolbar
    this->setContentsMargins(0,0,0,0);
    this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    // install event filter
    //QCoreApplication::instance()->installEventFilter(this);
}

void QPixmapToolTip::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);

    if (pix.isNull())
    {
        painter.setBrush(QBrush(QColor(255,0,0)));
        painter.drawRect(event->rect());
    }


    painter.setRenderHint(QPainter::Antialiasing);

    QSize pixSize = pix.size();
    pixSize.scale(event->rect().size(), Qt::KeepAspectRatio);

    QPixmap scaledPix = pix.scaled(pixSize,
                                   Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation
                                   );

    painter.drawPixmap(QPoint(), scaledPix);

}
