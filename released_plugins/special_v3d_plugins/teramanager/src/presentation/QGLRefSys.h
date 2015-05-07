#ifndef QGLREFSYS_H
#define QGLREFSYS_H

#include <QGLWidget>
#include "../control/CPlugin.h"

class teramanager::QGLRefSys : public QGLWidget
{
    Q_OBJECT

    private:

        int xRot;               //rotation angle (in degrees) along the X axis
        int yRot;               //rotation angle (in degrees) along the Y axis
        int zRot;               //rotation angle (in degrees) along the Z axis
        float xDim;             //x dimension (between 0.0 and 1.0)
        float yDim;             //y dimension (between 0.0 and 1.0)
        float zDim;             //z dimension (between 0.0 and 1.0)
        float ROIxDim;          //ROI x dimension (between 0.0 and 1.0)
        float ROIyDim;          //ROI y dimension (between 0.0 and 1.0)
        float ROIzDim;          //ROI z dimension (between 0.0 and 1.0)
        float ROIxShift;        //ROI x shift (between 0.0 and 1.0)
        float ROIyShift;        //ROI y shift (between 0.0 and 1.0)
        float ROIzShift;        //ROI z shift (between 0.0 and 1.0)
        QPoint lastPos;         //previous location of the mouse cursor to determine how much the object in the scene should be rotated, and in which direction
        double zoom;
        bool filled;            //equivalent to draw parallelepipedon faces (=true) or lines only (=false)

    public:

        QGLRefSys(QWidget *parent = 0);
        ~QGLRefSys();

        QSize minimumSizeHint() const;
        QSize sizeHint() const;
        int getXRot(){return xRot;}
        int getYRot(){return yRot;}
        int getZRot(){return zRot;}
        void setDims(int dimX, int dimY, int dimZ, int _ROIxDim=0, int _ROIyDim=0, int _ROIzDim=0, int _ROIxShift=0, int _ROIyShift=0, int _ROIzShift=0);
        void setFilled(bool _filled){filled = _filled; updateGL();}
        void setZoom(double _zoom){zoom = _zoom;}
        void resetZoom(){zoom = -15.0; updateGL();}


    public slots:

        // can be connected to a signal to syncronize rotation
        void setXRotation(int angle);
        void setYRotation(int angle);
        void setZRotation(int angle);

    signals:

        // signals emitted when rotation change
        void xRotationChanged(int angle);
        void yRotationChanged(int angle);
        void zRotationChanged(int angle);

        void mouseReleased();

    protected:

        // OpenGL initialization, viewport resizing, and painting methods MUST be overriden
        void initializeGL();
        void paintGL();
        void resizeGL(int width, int height);

        // Qt mouse event handlers to control rotation and zoom-in/out with the mouse
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);
        void wheelEvent(QWheelEvent *event);

        int heightForWidth( int w ) { return w; }
};

#endif // QGLREFSYS_H
