#ifndef QARROWBUTTON_H
#define QARROWBUTTON_H

#include <QPushButton>
#include "control/CPlugin.h"

class teramanager::QArrowButton : public QPushButton
{
    Q_OBJECT

    private:

        QColor arrColor;           //arrow color
        int arrSize;               //arrow size
        int arrwidth;              //arrow width;
        int margin;                //margin between arrow and button border
        int orientation;           //orientation (left-to-right or right-to-left)

        int transparency;          //trasparency [0,255]
        float scalePressed;        //arrow will be scale to this factor when mouse is pressed
        bool mouseOver;            //true when mouse is within button
        bool mousePressed;         //true when mouse is pressed within button

        //prevents from using the default constructor
        QArrowButton() : QPushButton(0){}
        QArrowButton(QWidget *parent = 0) : QPushButton(parent){}

    public:

        //the only available public constructor
        QArrowButton(QWidget *parent, QColor arrowColor, int arrowSize, int arrowWidth, int arrowMargin, int arrowOrientation);

        void leaveEvent(QEvent * e)
        {
            mouseOver = false;
            QPushButton::leaveEvent(e);
        }
        void enterEvent(QEvent * e)
        {
            mouseOver = true;
            QPushButton::enterEvent(e);
        }
        void mousePressEvent(QMouseEvent *e)
        {
            mousePressed = true;
            QPushButton::mousePressEvent(e);
        }
        void mouseReleaseEvent(QMouseEvent *e)
        {
            mousePressed = false;
            QPushButton::mouseReleaseEvent(e);
        }

    protected:

        //needed to draw the arrow with QPainter
        void paintEvent(QPaintEvent * evt);
};

#endif // QARROWBUTTON_H
