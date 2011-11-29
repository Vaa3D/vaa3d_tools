#include "vsvr.h"

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>

class QtLogo;

//! [0]
class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();

	void reSetView();
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

	void loadTiff(QString);
	void loadTexture(unsigned char* img, int width, int height, int depth, int channels);
	void draw_cube();
//! [0]

//! [1]
public slots:
    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);

signals:
    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);
//! [1]

//! [2]
protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);
//! [2]

//! [3]
private:
    //QtLogo *logo;
    int xRot;
    int yRot;
    int zRot;
	float xMove;
	float yMove;
	float scale;
    QPoint lastPos;
    QColor qtGreen;
    QColor qtPurple;


	VSVR vsvr;

	int tex_ni;
	int tex_nj;
	int tex_nk;

	float* tex;

	float xmin;
	float xmax;
	float ymin;
	float ymax;
	float zmin;
	float zmax;

	bool force_reload;
};
//! [3]

#endif
