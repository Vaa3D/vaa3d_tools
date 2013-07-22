#include "QGradientBar.h"

using namespace teramanager;

void QGradientBar::paintEvent(QPaintEvent * evt)
{
    //creatin QPainter object, enabling antialising and setting brush color differently if button is enabled or not
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if(isEnabled() && nSteps != -1)
    {
        //drawing gradient
        QLinearGradient fade(0, 0, width(), height());
        fade.setColorAt(0.0, QColor(0,      0,      255));
        fade.setColorAt(0.333, QColor(0,      255,    255));
        fade.setColorAt(0.666, QColor(255,    255,    0));
        fade.setColorAt(1.0, QColor(255,    0,      0));
        painter.fillRect(2, 2, width()-4, height()-4, fade);

        //drawing white bar
        painter.setBrush(QColor(255, 255, 255));
        painter.setPen(QColor(0, 0, 0, 0));
        painter.drawRect(static_cast<int>(  (step+1)*width()/float(nSteps)  +0.5f), 0,  static_cast<int>(  width()-(step+1)*width()/float(nSteps)  +0.5f), height());

        //drawing border
        painter.setBrush(QColor(255, 255, 255, 0));
        painter.setPen(QColor(0, 0, 0));
        painter.drawRect(0, 0, width(), height());
    }
    else
    {
        painter.setBrush(QColor(0, 0, 0, 0));
        painter.setPen(Qt::gray);
        painter.drawRect(0, 0, width(), height());
    }
}
