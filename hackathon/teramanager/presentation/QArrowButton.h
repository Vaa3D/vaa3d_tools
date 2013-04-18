#ifndef QARROWBUTTON_H
#define QARROWBUTTON_H

#include <QPushButton>
#include "control/CPlugin.h"

class teramanager::QArrowButton : public QPushButton
{
    Q_OBJECT

    private:

        QColor arrColor;  //arrow color
        int arrSize;               //arrow size
        int arrwidth;              //arrow width;
        int margin;                //margin between arrow and button border
        int orientation;           //orientation (left-to-right or right-to-left)

        //prevents from using the default constructor
        QArrowButton() : QPushButton(0){}
        QArrowButton(QWidget *parent = 0) : QPushButton(parent){}

    public:

        //the only available public constructor
        QArrowButton(QWidget *parent, QColor arrowColor, int arrowSize, int arrowWidth, int arrowMargin, int arrowOrientation);

    protected:

        //needed to draw the arrow with QPainter
        void paintEvent(QPaintEvent * evt);
};

#endif // QARROWBUTTON_H
