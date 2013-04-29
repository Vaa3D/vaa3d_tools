#include "QLineTree.h"

using namespace teramanager;

QLineTree::QLineTree(QWidget *parent, QColor _color, float _lineWidth, int _branches, int _vertMargin) : QWidget(parent)
{
    color = _color;
    lineWidth = _lineWidth;
    branches = _branches;
    verticalMargin = _vertMargin;
}

void QLineTree::paintEvent(QPaintEvent * evt)
{
    //creatin QPainter object, enabling antialising and setting brush color differently if button is enabled or not
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if(isEnabled())
        painter.setPen(QPen(color, lineWidth));
    else
        painter.setPen(QPen(Qt::gray, lineWidth));

    painter.drawLine(width(), verticalMargin, width(), height()-verticalMargin);
    painter.drawLine(0, verticalMargin, width(), verticalMargin);
    painter.drawLine(0, height()-verticalMargin, width(), height()-verticalMargin);
    for(int i=1; i<branches-1; i++)
        painter.drawLine(0, static_cast<int>(height()*i/(branches-1.0f)+0.5f), width(), static_cast<int>(height()*i/(branches-1.0f)+0.5f));

}
