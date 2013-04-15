#ifndef QARROWBUTTON_H
#define QARROWBUTTON_H

#include <QPushButton>
#include "control/CPlugin.h"

class teramanager::QArrowButton : public QPushButton
{
    Q_OBJECT

    private:

        Qt::GlobalColor arrColor;  //arrow color
        int arrSize;               //arrow size
        int arrwidth;              //arrow width;
        int margin;                //margin between arrow and button border

        //prevents from using the default constructor
        QArrowButton() : QPushButton(0){}
        QArrowButton(QWidget *parent = 0) : QPushButton(parent){}

    public:

        //the only available public constructor
        QArrowButton(QWidget *parent, Qt::GlobalColor arrowColor, int arrowSize, int arrowWidth, int arrowMargin);

    protected:

        //needed to draw the arrow with QPainter
        void paintEvent(QPaintEvent * evt);
};

#endif // QARROWBUTTON_H
