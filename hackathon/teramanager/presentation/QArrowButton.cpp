#include "QArrowButton.h"

using namespace teramanager;

QArrowButton::QArrowButton(QWidget *parent, Qt::GlobalColor arrowColor, int arrowSize, int arrowWidth, int arrowMargin) : QPushButton(parent)
{
    arrColor = arrowColor;
    arrSize = arrowSize;
    arrwidth = arrowWidth;
    margin = arrowMargin;
}

void QArrowButton::paintEvent(QPaintEvent * evt)
{
    QPushButton::paintEvent(evt);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);



    if(isEnabled())
        painter.setBrush(arrColor);
    else
        painter.setBrush(Qt::gray);

    //creating arrow head
    QPolygonF arrowHead;
    QPointF p1(margin, height()/2);
    QPointF p2(width()-margin, height()/2);
    QLineF line(p2, p1);
    double angle = ::acos(line.dx() / line.length());
    QPointF arrowP1 = p2 + QPointF(sin(angle + teramanager::pi / 3) * arrSize,
                                   cos(angle + teramanager::pi / 3) * arrSize);
    QPointF arrowP2 = p2 + QPointF(sin(angle + teramanager::pi - teramanager::pi / 3) * arrSize,
                                   cos(angle + teramanager::pi - teramanager::pi / 3) * arrSize);
    arrowHead << p2 << arrowP1 << arrowP2;

    //creating arrow body
    QRectF arrowBody(QPointF(margin, height()/2-arrwidth/2), QSizeF(arrowHead.boundingRect().x(), arrwidth));

    painter.drawRect(arrowBody);
    painter.drawPolygon(arrowHead);
}
