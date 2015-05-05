#include "qlabelimage.h"

QLabelImage::QLabelImage(QWidget *parent) :
    QLabel(parent)
{
    mScaleFactor = 1.0;
    setZoomLimits( 0.1, 10 );
}
