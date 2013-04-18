#include "QArrowButton.h"

using namespace teramanager;

QArrowButton::QArrowButton(QWidget *parent, QColor arrowColor, int arrowSize, int arrowWidth, int arrowMargin, int arrowOrientation) : QPushButton(parent)
{
    arrColor = arrowColor;
    arrSize = arrowSize;
    arrwidth = arrowWidth;
    margin = arrowMargin;
    orientation = arrowOrientation;
}

void QArrowButton::paintEvent(QPaintEvent * evt)
{
    //calling parent "paintEvent" method so that standard QPushButton is drawn
    QPushButton::paintEvent(evt);

    //creatin QPainter object, enabling antialising and setting brush color differently if button is enabled or not
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if(isEnabled())
    {
        painter.setBrush(arrColor);
        painter.setPen(arrColor);
        painter.setOpacity(0.8);
    }
    else
    {
        painter.setBrush(Qt::gray);
        painter.setPen(Qt::gray);
        painter.setOpacity(0.5);
    }

    //creating arrow head and body for the two orientations considered (left-to-right and right-to-left)
    QPolygonF arrowHead;
    QPointF p1(margin, height()/2);
    QPointF p2(width()-margin, height()/2);
    if(orientation == Qt::LeftToRight)
    {
        //arrow head
        QLineF line(p2, p1);
        double angle = ::acos(line.dx() / line.length());
        QPointF arrowP1 = p2 + QPointF(sin(angle + teramanager::pi / 3) * arrSize,
                                       cos(angle + teramanager::pi / 3) * arrSize);
        QPointF arrowP2 = p2 + QPointF(sin(angle + teramanager::pi - teramanager::pi / 3) * arrSize,
                                       cos(angle + teramanager::pi - teramanager::pi / 3) * arrSize);
        arrowHead << p2 << arrowP1 << arrowP2;

        //arrow body
        QRectF arrowBody(QPointF(margin, height()/2-arrwidth/2), QSizeF(arrowHead.boundingRect().x()-margin, arrwidth));

        //drawing arrow
        painter.drawRect(arrowBody);
        painter.drawPolygon(arrowHead);
    }
    else
    {
        //arrow head
        QLineF line(p1, p2);
        double angle = ::acos(line.dx() / line.length());
        QPointF arrowP1 = p1 + QPointF(sin(angle + teramanager::pi / 3) * arrSize,
                                       cos(angle + teramanager::pi / 3) * arrSize);
        QPointF arrowP2 = p1 + QPointF(sin(angle + teramanager::pi - teramanager::pi / 3) * arrSize,
                                       cos(angle + teramanager::pi - teramanager::pi / 3) * arrSize);
        arrowHead << p1 << arrowP1 << arrowP2;

        //arrow body
        QRectF arrowBody(QPointF(arrowHead.boundingRect().bottomRight().x(), height()/2-arrwidth/2), QSizeF(width() - 2*margin - arrowHead.boundingRect().width(), arrwidth));

        //drawing arrow
        painter.drawRect(arrowBody);
        painter.drawPolygon(arrowHead);
    }

}
