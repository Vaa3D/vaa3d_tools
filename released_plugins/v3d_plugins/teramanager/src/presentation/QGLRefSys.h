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
        QPoint lastPos;         //previous location of the mouse cursor to determine how much the object in the scene should be rotated, and in which direction

    public:

        QGLRefSys(QWidget *parent = 0);
        ~QGLRefSys();

        QSize minimumSizeHint() const;
        QSize sizeHint() const;
        int getXRot(){return xRot;}
        int getYRot(){return yRot;}
        int getZRot(){return zRot;}

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

        // Qt mouse event handlers to control rotation with the mouse
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);

        int heightForWidth( int w ) { return w; }
};

#endif // QGLREFSYS_H
