 #ifndef GLWIDGET_H
 #define GLWIDGET_H

 #include <QGLWidget>
 class GLWidget : public QGLWidget
 {
/******************************************
 * Basic Framework Starts
 ******************************************/
     Q_OBJECT

 public:
	 GLWidget(QWidget *parent = 0);
	 //GLWidget(unsigned int _width, unsigned int _height, unsigned int _depth,QWidget *parent = 0);
     ~GLWidget();
	 void reSetView();
	 //Set the minimum Size of GLWidget
	 QSize minimumSizeHint() const;
	 //Set the recommended Size of GLWidget
     QSize sizeHint() const; 
	 void getProjection(double & winX, double & winY, double &winZ, double objX, double objY, double objZ);
 protected:
     void initializeGL();
     void resizeGL(int width, int height);
	 //mousePressEvent is the virtual protected member function of QWidget
     void mousePressEvent(QMouseEvent *event);
	 //mouseMoveEvent is the virtual protected member function of QWidget
	 void mouseReleaseEvent(QMouseEvent *event);
     void mouseMoveEvent(QMouseEvent *event);
	 void mouseWheelEvent(QMouseEvent *event);
	 //mouseWheelEvent is a callback function define by myself;
	 void wheelEvent(QWheelEvent *event);
	 //paintGL will change frequently according to the input data format
	 void paintGL();
 private:
     void normalizeAngle(int *angle);
	 QPoint lastPos; 
	 QPoint pressPos;
	 
	 ///Signals and slots
 signals:
     void xRotationChanged(int angle);
     void yRotationChanged(int angle);
     void zRotationChanged(int angle);
	 void mouseClicked(float posX, float posY); 
public slots:
     void setXRotation(int angle);
     void setYRotation(int angle);
     void setZRotation(int angle);
 protected:
     int xRot;
     int yRot;
     int zRot;
	 float xMove;   //x move offset: range from -inf to +inf
	 float yMove;   //y move offset:range from -inf to +inf
	 double scale;
	 //unsigned int w;  //the width of 3D view : assigned to alpha * g_width
	 //unsigned int h;  //the height of 3D view: assigned to alpha * g_height
	 //unsigned int d;  //the depth of 3D view: assigned to alpha * g_depth
	 float ratio; //the ratio
	 float lenX;
	 float lenY;
	 float lenZ;
	 //max(ratioX,rationY,ratioZ)=1.0
	 
/******************************************
 * Basic Framework Ends
 ******************************************/

/******************************************
 * User Defination Starts
 *
 * Including data and operations
 ******************************************/
	 ///Member Functions
 public:
	 ///Member Variable
	 void loadTexture(unsigned char* tex,int w, int h, int d, int c);
	 unsigned char* m_texture;
	 int m_width;
	 int m_height;
	 int m_depth;
	 int m_channels;
/*****************************************
 * User Defination Ends
 *****************************************/
 };

 #endif

