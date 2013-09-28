#include "QArrowButton.h"

using namespace teramanager;

QArrowButton::QArrowButton(QWidget *parent, QColor arrowColor, int arrowSize, int arrowWidth, int arrowMargin, int arrowOrientation, bool _mousePressedBlocking) : QPushButton(parent)
{
    arrColor = arrowColor;
    arrSize = arrowSize;
    arrwidth = arrowWidth;
    margin = arrowMargin;
    orientation = arrowOrientation;
    transparency = 130;
    scalePressed = 0.7;
    mouseOver = false;
    mousePressed = false;
    mousePressedBlocking = _mousePressedBlocking;
    active = true;
}

void QArrowButton::paintEvent(QPaintEvent * evt)
{
    //calling parent "paintEvent" method so that standard QPushButton is drawn
    //QPushButton::paintEvent(evt);

    //creatin QPainter object, enabling antialising and setting brush color differently if button is enabled or not
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if(isEnabled())
    {
        if(mouseOver)
        {
            painter.setBrush(QColor(arrColor.red(), arrColor.green(), arrColor.blue(), transparency));
            painter.setPen(QColor(arrColor.red(), arrColor.green(), arrColor.blue(), transparency));
        }
        else
        {
            painter.setBrush(QColor(arrColor.red(), arrColor.green(), arrColor.blue()));
            painter.setPen(QColor(arrColor.red(), arrColor.green(), arrColor.blue()));
        }
    }
    else
    {
        painter.setBrush(Qt::gray);
        painter.setPen(Qt::gray);
        painter.setOpacity(0.5);
    }

    //creating arrow head and body for the two orientations considered (left-to-right and right-to-left)
    QPolygonF arrow, arrowHead;
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

        //arrow
        arrow << p2 << arrowP2 << arrowBody.bottomRight() << arrowBody.bottomLeft() << arrowBody.topLeft() << arrowBody.topRight() << arrowP1;
        if(!mousePressed)
            painter.drawPolygon(arrow);
        else
        {
            QTransform trans;
            trans=trans.scale(scalePressed,scalePressed);
            QPolygonF qpf2=trans.map(arrow);
            qpf2.translate(arrow.boundingRect().center()-qpf2.boundingRect().center());
            painter.drawPolygon(qpf2);
        }
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

        //arrow
        arrow << p1 << arrowP2 << arrowBody.topLeft() << arrowBody.topRight() << arrowBody.bottomRight() << arrowBody.bottomLeft() << arrowP1;
        if(!mousePressed)
            painter.drawPolygon(arrow);
        else
        {
            QTransform trans;
            trans=trans.scale(scalePressed,scalePressed);
            QPolygonF qpf2=trans.map(arrow);
            qpf2.translate(arrow.boundingRect().center()-qpf2.boundingRect().center());
            painter.drawPolygon(qpf2);
        }
    }

}
