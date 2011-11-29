#include "glwidget_ex.h"
#include <cassert>
#include <iostream>
using namespace std;

GLWidgetEX::GLWidgetEX(QWidget* parent) : GLWidget(parent)
{
}

GLWidgetEX::~GLWidgetEX()
{
}

void GLWidgetEX::setWidgetSize(int width, int height, int depth)
{
	m_width = width;
	m_height = height;
	m_depth = depth;
	float M = max(max(m_width,m_height),m_depth);
	ratio = 1/M;
	lenX = m_width*ratio;
	lenY = m_height*ratio;
	lenZ = m_depth*ratio;
}

void GLWidgetEX::addTrajectory(vector<float> & trajectory, int color)
{
	m_trajectorys.push_back(trajectory);
	m_colors.push_back(color);
}

void GLWidgetEX::paintGL()
{
	cout<<"track number : "<<m_trajectorys.size()<<endl;
	if(m_trajectorys.empty())
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
	 * Step 5 : Draw the trajectorys in 3D
	 *****************************************************/
	//glTranslatef(0.0,-1.0, 0.0);
	glTranslatef(0.5*lenX,     -0.5*lenY,        -0.5*lenZ);
	/*           |          |           |
				 move     to left    to bottom    to outside
				 */
	// 1. draw cell body
	float shiftx = - 0.5* m_width * ratio;
	float shifty = - 0.5* m_height * ratio;
	float shiftz = - 0.5* m_depth * ratio;
	for(int i = 0; i < m_trajectorys.size(); i++)
	{
		vector<float>& trajectory = m_trajectorys[i];
		int color = m_colors[i];
		red = color % 256;
		green = color / 256 % 256;
		blue = color / 256 / 256 % 256;
		assert(trajectory.size() % 3 == 0);
		int cell_num = trajectory.size() / 3;
		glPointSize(5.0);
		glColor3f(red/255.0, green/255.0, blue/255.0);
		glBegin(GL_POINTS);
		for(int j= 0; j < cell_num; j++)
		{
			float cx = trajectory[3*j];
			float cy = trajectory[3*j + 1];
			float cz = trajectory[3*j + 2];
			fx = cx * ratio + shiftx;
			fy = cy * ratio + shifty;
			fz = cz * ratio + shiftz;
			glVertex3f(fx, fy, fz);
		}
		glEnd();
		glPointSize(1.0);
		glColor3f(1.0 - red/255.0, 1.0 - green/255.0, 1.0 - blue/255.0);
		glBegin(GL_LINE_STRIP);
		for(int j = 0; j < cell_num; j++)
		{
			fx = trajectory[3*j]*ratio + shiftx;
			fy = trajectory[3*j + 1]*ratio + shifty;
			fz = trajectory[3*j + 2]*ratio + shiftz;
			glVertex3f(fx,fy,fz);
		}
		glEnd();
	}
}
