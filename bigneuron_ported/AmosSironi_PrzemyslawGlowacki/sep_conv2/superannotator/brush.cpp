#include "brush.h"

#include <QColor>
#include <QtGlobal>

SizedBrush::SizedBrush()
{
    this->width = 10;
    this->height = 10;
    this->depth = 3;
}

void SizedBrush::setSize(int width, int height, int depth)
{
    this->width = width;
    this->height = height;
    this->depth = depth;
}

PixelBrush::PixelBrush()
{

}

void PixelBrush::paint(QImage &qimg,
                        int x, int y, QColor highlightColor)
{
    const qreal opacity = 0.6;
    const qreal invOpacity = 0.99 - opacity;

    const qreal cRd = highlightColor.redF() * opacity;
    const qreal cGr = highlightColor.greenF() * opacity;
    const qreal cBl = highlightColor.blueF() * opacity;

    //TODO maybe skip rows by shifting i=0,j=0...
    if (x < 0 || y < 0)
        return;
    if (x >= qimg.width() || y >= qimg.height())
        return;
    QColor pixColor = QColor::fromRgb( qimg.pixel(x, y) );

    qreal r = pixColor.redF() * invOpacity + cRd;
    qreal g = pixColor.greenF() * invOpacity + cGr;
    qreal b = pixColor.blueF() * invOpacity + cBl;

    QRgb qrgb =  QColor::fromRgbF( r, g, b ).rgb();

    qimg.setPixel( x, y, qrgb);

}

void PixelBrush::paint(Matrix3D<LabelType> &data, int x, int y, int z, LabelType label)
{
    data.set(x, y, z, label);
}


CubeBrush::CubeBrush(){
//using superclass constructor
}

CubeBrush::CubeBrush(int width,int height, int depth)
{
    this->width = width;
    this->height = height;
    this->depth = depth;
}

//TODO maybe template this or similar
void CubeBrush::paint(QImage &qimg,
                        int x, int y, QColor highlightColor)
{
    const qreal opacity = 0.6;
    const qreal invOpacity = 0.99 - opacity;

    const qreal cRd = highlightColor.redF() * opacity;
    const qreal cGr = highlightColor.greenF() * opacity;
    const qreal cBl = highlightColor.blueF() * opacity;

    for(int i = x-width;i<x+width;i++) {
        for(int j = y-height; j<y+height; j++) {
                //TODO maybe skip rows by shifting i=0,j=0...
                if (i < 0 || j < 0)
                    continue;
                if (i >= qimg.width() || j >= qimg.height())
                    continue;
                QColor pixColor = QColor::fromRgb( qimg.pixel(i, j) );

                qreal r = pixColor.redF() * invOpacity + cRd;
                qreal g = pixColor.greenF() * invOpacity + cGr;
                qreal b = pixColor.blueF() * invOpacity + cBl;

                QRgb qrgb =  QColor::fromRgbF( r, g, b ).rgb();

                qimg.setPixel( i, j, qrgb);

        }
    }
}

void CubeBrush::paint(Matrix3D<LabelType> &data,
                        int x, int y, int z,
                        LabelType label)
{
    for(int i = x-width;i<x+width;i++) {
        for(int j = y-height; j<y+height; j++) {
            for(int k = z-depth; k<z+depth; k++)
            {
                //TODO maybe skip rows by shifting i=0,j=0...
                if (i < 0 || j < 0 || k < 0)
                    continue;
                if (i >= data.width() || j >= data.height()
                  ||k >= data.depth())
                    continue;

                 data.set(i, j, k, label);
            }
        }
    }
}

SphereBrush::SphereBrush(){
//using superclass constructor
}

SphereBrush::SphereBrush(int width,int height, int depth)
{
    this->width = width;
    this->height = height;
    this->depth = depth;
}

//TODO maybe template this or similar
void SphereBrush::paint(QImage &qimg,
                        int x, int y,  QColor highlightColor)
{

    const qreal opacity = 0.6;
    const qreal invOpacity = 0.99 - opacity;

    const qreal cRd = highlightColor.redF() * opacity;
    const qreal cGr = highlightColor.greenF() * opacity;
    const qreal cBl = highlightColor.blueF() * opacity;
    for(int i = x - width;i < x+width; i++)
    {
        for(int j = y-height; j < y+height; j++)
        {
            //TODO maybe skip rows by shifting i=0,j=0...
            if (i < 0 || j < 0)
                continue;
            if (i >= qimg.width() || j >= qimg.height())
                continue;

            if ( (i-x)*(i-x)/( (double)(width*width) )
               + (j-y)*(j-y)/( (double)(height*height) ) <= 1)
            {
                QColor pixColor = QColor::fromRgb( qimg.pixel(i, j) );

                qreal r = pixColor.redF() * invOpacity + cRd;
                qreal g = pixColor.greenF() * invOpacity + cGr;
                qreal b = pixColor.blueF() * invOpacity + cBl;

                QRgb qrgb =  QColor::fromRgbF( r, g, b ).rgb();
                qimg.setPixel( i, j, qrgb);
            }
        }
    }
}

void SphereBrush::paint(Matrix3D<LabelType> &data,
                        int x, int y, int z,
                        LabelType label)
{
    for(int i = x-width;i<x+width;i++) {
        for(int j = y-height; j<y+height; j++) {
            for(int k = z-depth; k<z+depth; k++)
            {
                //TODO maybe skip rows by shifting i=0,j=0...
                if (i < 0 || j < 0 || k < 0)
                    continue;
                if (i >= data.width() || j >= data.height()
                  ||k >= data.depth())
                    continue;

             if ( (i-x)*(i-x)/( (double)(width*width) )
                + (j-y)*(j-y)/( (double)(height*height) )
                + (k-z)*(k-z)/( (double)(depth*depth) ) <= 1)
                    data.set(i, j, k, label);
            }
        }
    }
}
