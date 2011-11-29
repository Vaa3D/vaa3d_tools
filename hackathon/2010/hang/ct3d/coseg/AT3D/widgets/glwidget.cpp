/*
 *  untitled.h
 *  
 *
 *  Created by XIAO Hang ,PH.D STUDENT,Group tangkun on 6/28/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <QtGui>
#include <QtOpenGL>

#include <math.h>
#include <map>
#include <iostream>
#include "glwidget.h"
using namespace std;

GLWidget::GLWidget(QWidget *parent)
:  QGLWidget(parent)
{
	xRot = 0;
	yRot = 0;
	zRot = 0;
	xMove = 0.0;
	yMove = 0.0;
	scale = 1.0; //Make the initial view larger
	m_texture = NULL;
	m_width = 0;
	m_height = 0;
	m_depth = 0;
	m_channels = 0;
}

GLWidget::~GLWidget()
{
	makeCurrent();
}

void GLWidget::reSetView()
{
	xRot = 0;
	yRot = 0;
	zRot = 0;
	xMove = 0.0;
	yMove = 0.0;
	scale = 1.0; //Make the initial view larger
}

QSize GLWidget::minimumSizeHint() const
{
	return QSize(312, 499);
}

QSize GLWidget::sizeHint() const
{
	return QSize(312*2, 499*2);
}

void GLWidget::initializeGL()
{
	qglClearColor(QColor(25,25,63)/*Qt::blue trolltechPurple.dark()*/);
	glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glEnable(GL_POINT_SMOOTH);
}

void GLWidget::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
}

void GLWidget::getProjection(double & winX, double & winY, double &winZ, double objX, double objY, double objZ)
{
	double shiftx = - 0.5* m_width * ratio;
	double shifty = - 0.5* m_height * ratio;
	double shiftz = - 0.5* m_depth * ratio;

	objX = objX * ratio + shiftx;
	objY = objY * ratio + shifty;
	objZ = objZ * ratio + shiftz;

	GLdouble modeview[16];
	GLdouble projection[16];
	GLint viewport[4];

	glGetDoublev(GL_MODELVIEW_MATRIX,modeview);
	glGetDoublev(GL_PROJECTION_MATRIX,projection);
	glGetIntegerv(GL_VIEWPORT,viewport);

	gluProject(objX, objY, objZ, modeview, projection, viewport, &winX, &winY, &winZ);
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if(pressPos == event->pos())
	{
		float winX = (float)( event->x());  // horizental distance from center of the viewport, left(-1) to right (1)
		float winY = (float)(this->height() - event->y());  // vertical   distance from center of the viewport, bottow(-1) to up (1)
		emit mouseClicked(winX, winY);
	}
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
	//Here is very important to record the initial mouse position 
	lastPos = event->pos();
	pressPos = lastPos;
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
	if (event->buttons() & Qt::LeftButton) {
		int dx = event->x() - lastPos.x();
		int dy = event->y() - lastPos.y();
		setXRotation(xRot + 8 * dy);
		setYRotation(yRot + 8 * dx);
	} 
	else if (event->buttons() & Qt::RightButton) {
		//event->pos returns point position in the widget
		//it should be converted to the position in the opengl coordinate system
		//Each step in GLWidget is equal to scale/2.0 that of steps in 3D view
		float ratio = 2.0/scale;
		xMove += ratio * (float)(event->x() - lastPos.x())/this->size().width();
		yMove -= ratio * (float)(event->y() - lastPos.y())/this->size().height();
		updateGL();
	}
	lastPos = event->pos();
}

void GLWidget::wheelEvent(QWheelEvent * event)
{
	int wheelSteps = event->delta() / 120;
	scale = scale * pow(1.1,wheelSteps);
	event->accept();
	updateGL();
}

void GLWidget::paintGL()
{
	if(m_texture == NULL)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		return;
	}

	/***********************************************
	 * Step 1: Initial Definition of Local Variable
	 ***********************************************/
	float fx=0,fy=0,fz=0; //Point(fx,fy,fz)
	//Color(r,g,b)
	int red=0,green =0, blue =0;

	/******************************************************
	 * Step 2: Set openGL Environment and  Do Scale,Translate as well as Rotation Operation
	 *
	 * Becareful :
	 * 1. The coordinate system origin locate in the center of the plane
	 *    x : increase from left to right
	 *    y : increase from bottom to top
	 *    z : increase from inside to outside
	 *    The bottom left is (-1,-1,0);
	 *    The top right is (1,1,0);
	 *
	 * 2. The GLWidget's origin is located on top left
	 *   x: increase from left to right
	 *   y: increase from top to bottom
	 *
	 * 3. The origin of the 3D image we want to show, locate on bottom left outside
	 *    x : increase from left to right
	 *    y : increase from bottom to top
	 *    z : increase from outside to inside
	 -----------------
	 /|                /|
	 / |               / |
	 -----------------   |
	 |  |      y       |  |
	 |  |      ^       |  |
	 |  |      |       |  |
	 |  |      o--->x  |  |
	 |  |     /        |  |
	 |  |              |  |
	 |   --------------|--      
	 | /               | /
	 |/                |/
	 ----------------- 
	 * Considerless about the move operation, we will draw our object 
	 * in the box(-0.5*lenX,-0.5*lenY,-0.5*lenZ) -> (0.5*lenX,0.5*lenY,0.5*lenZ)
	 * To rotation, we should first move the origin to (0, 0, 0). 	
	 */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glScalef(scale, scale, scale);
	//glPointSize(scale);
	//Right mouse will drag the orgin to (xMove,yMove,0.0)
	glTranslatef(xMove,yMove,0.0);
	//Move origin to 3D images's center to rotation 
	//glTranslatef(0.0,0.0,-0.5);
	glRotated(xRot / 16.0, 1.0, 0.0, 0.0);
	glRotated(yRot / 16.0, 0.0, 1.0, 0.0);
	glRotated(zRot / 16.0, 0.0, 0.0, 1.0);


	/********************************************
	 * Step 3: Draw Six surfaces
	 ********************************************/
	//Move the origin to top left
	glTranslatef(-0.5*lenX,     0.5*lenY,        0.5*lenZ);
	/*           |          |           
				 move     to left    to top 
				 */
	glColor3f(0.0,0.0,0.0);
	/*
	   ------------------
	   Z/|                 /|
	   / |                / |
	   -------X-->-------   |
	   |******************|  |
	   |******************|  |
	   |******************|  |
	   Y******************|  |
	   |******************|  |
	   ^******************|  |
	   |******************|--      
	   |******************| /
	   |******************|/
	   ---------<-------- 
	   */

	/* Plane 1 */
	// Draw front x-y plane
	glBegin(GL_QUADS);
	glVertex3f(0.0,0.0,0.0);
	glVertex3f(0.0,-1.0*lenY,0.0);
	glVertex3f(1.0*lenX,-1.0*lenY,0.0);
	glVertex3f(1.0*lenX,0.0,0.0);
	glEnd();

	/* Plane 2 */
	// Draw top x-z plane
	glBegin(GL_QUADS);
	glVertex3f(0.0,0.0,0.0);
	glVertex3f(1.0*lenX,0.0,0.0);
	glVertex3f(1.0*lenX,0.0,-1.0*lenZ);
	glVertex3f(0.0,0.0,-1.0*lenZ);
	glEnd();

	/* Plane 3 */
	// Draw left y-z plane
	glBegin(GL_QUADS);
	glVertex3f(0.0,0.0,0.0);
	glVertex3f(0.0,0.0,-1.0*lenZ);
	glVertex3f(0.0,-1.0*lenY,-1.0*lenZ);
	glVertex3f(0.0,-1.0*lenY,0.0);
	glEnd();

	/* Plane 4 */
	// Draw right y-z plane
	glBegin(GL_QUADS);
	glVertex3f(1.0*lenX,0.0,0.0);
	glVertex3f(1.0*lenX,-1.0*lenY,0.0);
	glVertex3f(1.0*lenX,-1.0*lenY,-1.0*lenZ);
	glVertex3f(1.0*lenX,0.0,-1.0*lenZ);
	glEnd();	

	/* Plane 5 */
	// Draw bottom x-z plane
	glBegin(GL_QUADS);
	glVertex3f(0.0,-1.0*lenY,0.0);
	glVertex3f(0.0,-1.0*lenY,-1.0*lenZ);
	glVertex3f(1.0*lenX,-1.0*lenY,-1.0*lenZ);
	glVertex3f(1.0*lenX,-1.0*lenY,0.0);
	glEnd();

	/* Plane 6 */
	// Draw back x-y plane
	glBegin(GL_QUADS);
	glVertex3f(0.0,0.0,-1.0*lenZ);
	glVertex3f(1.0*lenX,0.0,-1.0*lenZ);
	glVertex3f(1.0*lenX,-1.0*lenY,-1.0*lenZ);
	glVertex3f(0.0,-1.0*lenY,-1.0*lenZ);
	glEnd();

	/******************************************
	 * Step 4: Draw twelve axises
	 ******************************************/
	glLineWidth(3.0);
	/*
	   (0,0,-1)-------------(1,0,-1)
	   /|                  / |
	   / |                /   |
	   (0,0,0)------------(1,0,0)  |
	   |  |               |    |
	   |  |               |    |
	   |  |               |    |
	   |  |               |    |
	   |  |               |    |
	   |  |               |    |
	   |(0,-1,-1)---------|-(1,-1,-1)
	   | /                |   /
	   |/                 | /
	   (0,-1,0)-----------(1,-1,0)
	   */

	glBegin(GL_LINES);
	glColor3f(1.0,0.0,0.0);
	glVertex3f(0.0,0.0,-1.0*lenZ);
	glVertex3f(1.1*lenX,0.0,-1.0*lenZ);
	glEnd();

	//Draw 
	glBegin(GL_LINES);
	glColor3f(0.0,1.0,0.0);
	glVertex3f(0.0,0.0,-1.0*lenZ);
	glVertex3f(0.0,-1.1*lenY,-1.0*lenZ);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0.0,0.0,1.0);
	glVertex3f(0.0,0.0,0.1*lenZ);
	glVertex3f(0.0,0.0,-1.0*lenZ);
	glEnd();

	glLineWidth(2.0);
	glBegin(GL_LINES);
	glColor3f(0.5,0.5,0.5);
	glVertex3f(0.0,0.0,0);
	glVertex3f(1.0*lenX,0.0,0);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0.5,0.5,0.5);
	glVertex3f(0.0,0.0,0);
	glVertex3f(0.0,-1.0*lenY,0);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0.5,0.5,0.5);
	glVertex3f(0.0,0.0,0);
	glVertex3f(0.0,0.0,-1.0*lenZ);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0.5,0.5,0.5);
	glVertex3f(1.0*lenX,0.0,0.0);
	glVertex3f(1.0*lenX,0.0,-1.0*lenZ);
	glEnd();
	glBegin(GL_LINES);
	glColor3f(0.5,0.5,0.5);
	glVertex3f(1.0*lenX,0.0,0.0);
	glVertex3f(1.0*lenX,-1.0*lenY,0.0);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0.5,0.5,0.5);
	glVertex3f(1.0*lenX,0.0,0.0);
	glVertex3f(1.0*lenX,-1.0*lenY,0.0);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0.5,0.5,0.5);
	glVertex3f(1.0*lenX,-1.0*lenY,0.0);
	glVertex3f(0.0,-1.0*lenY,0.0);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0.5,0.5,0.5);
	glVertex3f(1.0*lenX,-1.0*lenY,0.0);
	glVertex3f(1.0*lenX,-1.0*lenY,-1.0*lenZ);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0.5,0.5,0.5);
	glVertex3f(0.0,-1.0*lenY,0.0);
	glVertex3f(0.0,-1.0*lenY,-1.0*lenZ);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0.5,0.5,0.5);
	glVertex3f(0.0,0.0,-1.0*lenZ);
	glVertex3f(1.0*lenX,0.0,-1.0*lenZ);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0.5,0.5,0.5);
	glVertex3f(0.0,0.0,-1.0*lenZ);
	glVertex3f(0.0,-1.0*lenY,-1.0*lenZ);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0.5,0.5,0.5);
	glVertex3f(1.0*lenX,-1.0*lenY,-1.0*lenZ);
	glVertex3f(0.0,-1.0*lenY,-1.0*lenZ);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0.5,0.5,0.5);
	glVertex3f(1.0*lenX,-1.0*lenY,-1.0*lenZ);
	glVertex3f(1.0*lenX,0.0,-1.0*lenZ);
	glEnd();

	/*****************************************************
	 * Step 5 : Draw the cells in 3D
	 *****************************************************/
	//glTranslatef(0.0,-1.0, 0.0);
	glTranslatef(0.5*lenX,     -0.5*lenY,        -0.5*lenZ);
	/*           |          |           |
				 move     to left    to bottom    to outside
				 */
	glBegin(GL_POINTS);
	// 1. draw cell body
	float shiftx = - 0.5* m_width * ratio;
	float shifty = - 0.5* m_height * ratio;
	float shiftz = - 0.5* m_depth * ratio;
	for(int i = 0; i < m_width * m_height * m_depth; i++)
	{
		if(m_texture[3*i] == 0 && m_texture[3*i+1] == 0 && m_texture[3*i+2] == 0) continue;
		red = m_texture[3*i];
		green = m_texture[3*i + 1];
		blue = m_texture[3*i + 2];
		glColor3f(red/255.0,green/255.0,blue/255.0);
		int d = (i / m_width / m_height) % m_depth;
		int h = (i / m_width) % m_height;
		int w = i % m_width;

		fx= w * ratio + shiftx ;
		fy= h * ratio + shifty;
		fz= d * ratio + shiftz;
		glVertex3f(fx ,fy,fz);
	}
	glEnd();
}

void GLWidget::normalizeAngle(int *angle)
{
	while (*angle < 0)
		*angle += 360 * 16;
	while (*angle > 360 * 16)
		*angle -= 360 * 16;
}



void GLWidget::setXRotation(int angle)
{
	normalizeAngle(&angle);
	if (angle != xRot) {
		xRot = angle;
		emit xRotationChanged(angle);
		updateGL();
	}
}

void GLWidget::setYRotation(int angle)
{
	normalizeAngle(&angle);
	if (angle != yRot) {
		yRot = angle;
		emit yRotationChanged(angle);
		updateGL();
	}
}

void GLWidget::setZRotation(int angle)
{
	normalizeAngle(&angle);
	if (angle != zRot) {
		zRot = angle;
		emit zRotationChanged(angle);
		updateGL();
	}
}

void GLWidget::loadTexture(unsigned char* texture, int w, int h, int d, int c)
{
	m_texture = texture;
	m_width = w;
	m_height = h;
	m_depth = d;
	m_channels = c;
	float M = max(max(m_width,m_height),m_depth);
	ratio = 1/M;
	lenX = m_width*ratio;
	lenY = m_height*ratio;
	lenZ = m_depth*ratio;

}
