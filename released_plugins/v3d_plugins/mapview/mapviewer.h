/* mapviewer.h
 * 2011-01-26 the program is created by Yang jinzhu
 */


#ifndef __MAPVIEWER__
#define __MAPVIEWER__

// maviewer
#include <qwidget.h>

#include <fstream>
#include <sstream>
#include <iostream>

#include <vector>
#include <list>
#include <bitset>

#include <set>

// reader 
#include "basic_surf_objs.h"
#include "stackutil-11.h"
#include "volimg_proc.h"
#include "img_definition.h"
#include "basic_landmark.h"
#include "basic_triview.h"
#include "mg_utilities.h"
#include "mg_image_lib11.h"
#include "basic_landmark.h"

#include "basic_4dimage.h"

#include "arthurwidgets.h"
// multithreads
#include <pthread.h>
// interface v3d
#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>
#include "colormap.h"

#include "y_imglib.h"

//#include "y_imglib.h"
//
#include <QBasicTimer>
#include <QPolygonF>
//#include <QMainWindow>
#include <qgroupbox.h>

#include "volimg_proc.h"
#include <iostream>

#include <QMutex>
#include <QQueue>
#include <QThread>

//class HoverPoints;
//class QLineEdit;
//class QLabel;
//class QScrollBar;
//class QSpinBox;
//class QRadioButton;
//class QPushButton;
//class QHBoxLayout;
//class QGridLayout;
//class QVBoxLayout;
//class QCheckBox;
//class QTextBrowser;
//class XMapView;

using namespace std;
enum ImagePlaneDisplayType {imgPlaneUndefined, imgPlaneX, imgPlaneY, imgPlaneZ};// define indexed data structures

template <class T> QPixmap copyRaw2QPixmap_xPlanes(const T * pdata, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, ImageDisplayColorType Ctype,V3DLONG cz0, V3DLONG cz1, V3DLONG cz2,double *p_vmax, double *p_vmin);
template <class T> QPixmap copyRaw2QPixmap_yPlanes(const T * pdata, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3,ImageDisplayColorType Ctype, V3DLONG cz0, V3DLONG cz1, V3DLONG cz2,double *p_vmax, double *p_vmin);
template <class T> QPixmap copyRaw2QPixmap_zPlanes(const T * pdata, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3,ImageDisplayColorType Ctype, V3DLONG cz0, V3DLONG cz1, V3DLONG cz2,double *p_vmax, double *p_vmin);
template <class T> QPixmap copyRaw2QPixmap(const T * pdata, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, ImageDisplayColorType Ctype,V3DLONG cz0, V3DLONG cz1, V3DLONG cz2,ImagePlaneDisplayType disType, double *p_vmax, double *p_vmin);

template <class T1, class T2> 
int CopyData_resamp_raw(T1 *apsInput, T2 *aspOutput,V3DLONG channel_size, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer, V3DLONG resampling_size);

template <class T1, class T2> 
int CopyData_resamp_tc(T1 *apsInput, T2 *aspOutput,V3DLONG * sz,V3DLONG * szo,
					   V3DLONG start_x, V3DLONG start_y, V3DLONG start_z,
				       V3DLONG x_start,V3DLONG y_start,V3DLONG z_start, V3DLONG x_end,V3DLONG y_end,V3DLONG z_end, 
					   V3DLONG tile2vi_xs, V3DLONG tile2vi_ys,V3DLONG tile2vi_zs,V3DLONG resampling_size);

template <class T> 
int CopyData(T *apsInput, T *aspOutput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer,V3DLONG channel_size);

template <class T>
int CopyData_tc(T *apsInput, T *aspOutput,V3DLONG * szo,
				V3DLONG start_x, V3DLONG start_y, V3DLONG start_z,
				V3DLONG x_start,V3DLONG y_start,V3DLONG z_start, V3DLONG x_end,V3DLONG y_end,V3DLONG z_end, 
				V3DLONG vx,V3DLONG vy,V3DLONG vz);


//template <class T> bool minMaxInVector(T * p, V3DLONG len, V3DLONG &pos_min, T &minv, V3DLONG &pos_max, T &maxv);


using namespace std;

//
class XMapView : public QWidget//, public V3DPluginInterface
{
    
	Q_OBJECT
	//Q_INTERFACES(V3DPluginInterface);
   // Q_PROPERTY(double scale READ scale WRITE changeScale)	
public:
	XMapView(QWidget *parent);
	~XMapView();
	
	//void setImgData(ImagePlaneDisplayType ptype,ImagePixelType dtype,V3DLONG *sz_compressed,V3DLONG cz0, V3DLONG cz1, V3DLONG cz2,unsigned char *pdata);
	
	void setImgData(ImagePlaneDisplayType ptype,ImagePixelType dtype,ImageDisplayColorType Ctype,V3DLONG *sz_compressed,V3DLONG cz0, V3DLONG cz1, V3DLONG cz2,V3DLONG xslicesize,V3DLONG yslicesize, V3DLONG zslicesize, unsigned char *pdata,double * p_vmax, double* p_vmin);

	void Setwidget(V3DPluginCallback &callback, QString m_FileName, QString curFilePathInput, float scaleFactorInput);
	
	void update_v3dviews(V3DPluginCallback *callback, long start_x, long start_y, long start_z, long end_x, long end_y, long end_z);
	
	int disp_width, disp_height;
	int disp_scale;
	
	bool flag_syn;
	bool b_xyview;
	bool b_yzview;
	bool b_zxview;
	QPoint dragStartPosition;
	
	QPoint dragEndPosition;
	
	long cx, cy, cz,cc;
	long cur_x, cur_y, cur_z;
	long plane_n;
	long start_x,start_y,start_z;
	long end_x,end_y,end_z;
	long in_startx ;
	long in_starty ;
	
	long in_endx ;
	long in_endy ;
	
	long in_xslicesize;
	long in_yslicesize;
	long in_zslicesize;
	
	bool b_creadWindow;
	
	V3DPluginCallback *callback1;
	
	QString curFilePath;
	
	int scaleFactor;
	
	static XMapView* m_show;
	
	int get_disp_width() {return disp_width;}
	
	int get_disp_height() {return disp_height;}
	
	double get_disp_scale() {return disp_scale;}
	
	void set_disp_width(int a) {disp_width = a;}
	void set_disp_height(int a) {disp_height = a;}
	void set_disp_scale(double a) {disp_scale = a; }
	
	int focusPlaneCoord() {return cur_focus_pos;}	
	double scale() const { return m_scale; }
    void drawPixmapType(QPainter *painter);	
	void paintEvent(QPaintEvent *event);
	
	void drawROI(QPainter *painter);
	
	void mousePressEvent(QMouseEvent *e);
    void mouseLeftButtonPressEvent(QMouseEvent *e);
    void mouseRightButtonPressEvent(QMouseEvent *e);
	void mouseMoveEvent (QMouseEvent * e);
	void wheelEvent(QWheelEvent * e);
    void enterEvent (QEvent * e);
    void leaveEvent (QEvent * e);
	 
	void mouseReleaseEvent(QMouseEvent * e);
	
	Y_VIM<float, long, indexed_t<long, float>, LUT<long> > vim;
	
public slots:
signals:
	void focusXChanged(int x_new);
    void focusYChanged(int y_new);
    void focusZChanged(int z_new);	
private:
	enum XFormType { VectorType, PixmapType, TextType};   
	double m_scale;
	
	QPointF curDisplayCenter;
	QPointF curDisplayCenter0;
	
	QPoint curMousePos;
	
	unsigned char *imagData;
	
	QCursor myCursor;	
	
	XFormType Gtype;	

	ImagePlaneDisplayType Ptype;
	
	ImagePixelType Datatype;
	
    QPixmap pixmap; //xy plane
    
	int cur_focus_pos;
	
	bool b_displayFocusCrossLine;
	bool b_mouseend;
	bool b_Lmouse;
	bool b_mousmove;
	bool bMouseCurorIn;
	
	int focusPosInWidth, focusPosInHeight;
	
	unsigned char *compressed;
	
	long roi_top, roi_left, roi_bottom, roi_right;
	//QImage sourceImage;
	long channel_compressed_sz;
	
	unsigned char *compressed1d;
	
	int mousenumber;
	
private:
};
// MAPViewer interface
class ImageSetWidget : public QWidget//,public QDialog 
{
	Q_OBJECT
	
public:
	 ImageSetWidget(V3DPluginCallback &callback, QWidget *parent,QString m_FileName, QString curFilePathInput, float scaleFactorInput,bool &b_shouw);
	
	void initialize();
	
	void update_v3dviews(V3DPluginCallback *callback, long start_x, long start_y, long start_z, long end_x, long end_y, long end_z);
	
	void createGUI();
	
	void update_triview();
	
	bool updateminmaxvalues();
	bool setCTypeBasedOnImageData();
	
	bool bcreadViews;
	
public slots:
	void updateGUI();
	void drawdata();
	
	void toggCreadViewCheckBox();
	

	
public:
//*********************************************************
	QGroupBox *dataGroup, *viewGroup, *infoGroup;
	
	QGroupBox *mainGroup, *coordGroup;
	
	QGroupBox *settingGroup;
	
	QScrollBar *xSlider, *ySlider, *zSlider;
	
	QSpinBox *xValueSpinBox, *yValueSpinBox, *zValueSpinBox;
	
	QLabel *xSliderLabel, *ySliderLabel, *zSliderLabel;
	
	QSpinBox *xSizeSpinBox, *ySizeSpinBox, *zSizeSpinBox;
	
	QLabel *xSizeLabel, *ySizeLabel, *zSizeLabel;
	
	QPushButton* dataCopyButton;
	
	QCheckBox *CreateViewCheckBox;
	
	QHBoxLayout *allLayout;
	
	QVBoxLayout *dataGroupLayout;
	
	QGridLayout *xyzViewLayout;
	
	QVBoxLayout *mainGroupLayout;
	
	QGridLayout *coordGroupLayout;
	
	QGridLayout *settingGroupLayout;
	
	QGridLayout *datacopyGroupLayout;
   
	V3DPluginCallback *callback1;
	 
	XMapView *xy_view; //change in Z
    XMapView *yz_view; //change in X
    XMapView *zx_view; //change in Y
	XMapView *mapview ;
	
	//******************
public:
	long cx, cy, cz, cc; // compressed data
	long cur_x, cur_y, cur_z;
	long sx,sy,sz;
	long channel_compressed_sz;
	
	long init_x, init_y, init_z; // control window
	
	long wx, wy, wz;
	
	long roi_start_x, roi_start_y, roi_start_z;
	
	long roi_end_x, roi_end_y, roi_end_z;
	
	long xslicesize,yslicesize,zslicesize;
	
	unsigned char *compressed1d;
	
	double * p_vmax, * p_vmin; //whole volume max/min values. Use pointer to handle multiple channels separately
	
	V3DLONG *sz_compressed;
	
	Image4DSimple p4DImage;
	
	ImagePixelType dtype;
	
	ImageDisplayColorType Ctype;
   
	QString curFilePath;
	
	float scaleFactor;
	// virtual image
	Y_VIM<float, long, indexed_t<long, float>, LUT<long> > vim;
	bool Bcopy;
	long flag_changed;
};

// interface v3d plugin
class MAPiewerPlugin : public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface);
	
// v3d interface	
public:
	QStringList menulist() const;
	
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);
	
	QStringList funclist() const {return QStringList();}
	
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}	
	
	void iViewer(V3DPluginCallback &callback, QWidget *parent);
	
	void resampling_tc(V3DPluginCallback &callback, QWidget *paren);

	void resampling_rawdata(V3DPluginCallback &callback, QWidget *parent);
	
	void tifdata_rawdata(V3DPluginCallback &callback, QWidget *parent);
	
	void loadtc_save(V3DPluginCallback &callback, QWidget *parent);
	
	Y_VIM<float, long, indexed_t<long, float>, LUT<long> > vim;
	
		
public:
	
};
class Mutthread_tiftoraw : public QThread
{
	//QT_THREAD_SUPPORT
	Q_OBJECT
public:
	Mutthread_tiftoraw(const QString filename, int n);
	void run();
	Y_VIM<float, long, indexed_t<long, float>, LUT<long> > vim;
	QString m_FileName;
};
// indexed data structure
class IndexedData
{
	Y_VIM<float, long, indexed_t<long, float>, LUT<long> > vim;
	
};


#endif



