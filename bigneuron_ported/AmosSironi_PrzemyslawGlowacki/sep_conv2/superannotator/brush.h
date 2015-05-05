#ifndef BRUSH_H
#define BRUSH_H

#include <QImage>

#include "Matrix3D.h"
#include "CommonTypes.h"

class Brush
{
public:
    virtual void paint(QImage &data, int x, int y, QColor highlightColor) = 0;
    virtual void paint(Matrix3D<LabelType> &data, int x, int y, int z, LabelType label) = 0;
    virtual ~Brush() {}
};

class PixelBrush : public Brush
{

public:
    PixelBrush();
    void paint(QImage &data, int x, int y, QColor highlightColor);
    void paint(Matrix3D<LabelType> &data, int x, int y, int z, LabelType label);

};

class SizedBrush : public Brush
{

public:
    int width;
    int height;
    int depth;

    SizedBrush();

    void setSize(int width, int height, int depth);

};

class CubeBrush : public SizedBrush
{
public:

    CubeBrush();

    CubeBrush(int width, int height, int depth);

    void paint(QImage &data, int x, int y, QColor highlightColor);

    void paint(Matrix3D<LabelType> &data, int x, int y, int z, LabelType label);

};

class SphereBrush : public SizedBrush
{
public:

    SphereBrush();

    SphereBrush(int width, int height, int depth);

    void paint(QImage &data, int x, int y, QColor highlightColor);

    void paint(Matrix3D<LabelType> &data, int x, int y, int z, LabelType label);

};


#endif // BRUSH_H
