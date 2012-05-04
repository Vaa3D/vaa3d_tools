#include <iostream>
#include <QtGui>
#include <v3d_interface.h>
#include "mapview.h"
#include "mapview_gui.h"

using namespace std;

MapViewWidget::MapViewWidget(V3DPluginCallback2 * _callback, Mapview_Paras _paras,  QWidget *parent) : QWidget(parent)
{
	callback = _callback; curwin = 0; paras = _paras;

	mapview.setPara(paras.hraw_dir, paras.L, paras.M, paras.N, paras.l, paras.m, paras.n, paras.channel);

	setWindowTitle("Mapview Control");
	/*setWindowFlags( Qt::Widget
	  | Qt::Tool
	  | Qt::CustomizeWindowHint | Qt::WindowTitleHint  //only title bar, disable buttons on title bar
	  );
	  */
	setFixedWidth(350);
	setFixedHeight(300);

	QGridLayout *layout = new QGridLayout(this);

	cutLeftXSlider = new QScrollBar(Qt::Horizontal);
	cutLeftXSlider->setRange(0, 99); 
	cutLeftXSlider->setSingleStep(1);
	cutLeftXSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	cutLeftXSlider->setValue(0);
	QLabel* cutLeftXSliderLabel = new QLabel("X-cut");

	cutRightXSlider = new QScrollBar(Qt::Horizontal);
	cutRightXSlider->setRange(1, 100); 
	cutRightXSlider->setSingleStep(1);
	cutRightXSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	cutRightXSlider->setValue(100);
	QLabel* cutRightXSliderLabel = new QLabel("");

	cutLeftYSlider = new QScrollBar(Qt::Horizontal);
	cutLeftYSlider->setRange(0, 99); 
	cutLeftYSlider->setSingleStep(1);
	cutLeftYSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	cutLeftYSlider->setValue(0);
	QLabel* cutLeftYSliderLabel = new QLabel("Y-cut");

	cutRightYSlider = new QScrollBar(Qt::Horizontal);
	cutRightYSlider->setRange(1, 100); 
	cutRightYSlider->setSingleStep(1);
	cutRightYSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	cutRightYSlider->setValue(100);
	QLabel* cutRightYSliderLabel = new QLabel("");

	cutLeftZSlider = new QScrollBar(Qt::Horizontal);
	cutLeftZSlider->setRange(0, 99); 
	cutLeftZSlider->setSingleStep(1);
	cutLeftZSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	cutLeftZSlider->setValue(0);
	QLabel* cutLeftZSliderLabel = new QLabel("Z-cut");

	cutRightZSlider = new QScrollBar(Qt::Horizontal);
	cutRightZSlider->setRange(1, 100); 
	cutRightZSlider->setSingleStep(1);
	cutRightZSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	cutRightZSlider->setValue(100);
	QLabel* cutRightZSliderLabel = new QLabel("");

	zoomSlider = new QScrollBar(Qt::Horizontal);
	zoomSlider->setRange(1, paras.level_num); 
	zoomSlider->setSingleStep(1);
	zoomSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	zoomSlider->setValue(1); // set minimum zoom, the thumbnail whole image would display
	QLabel* zoomSliderLabel = new QLabel("zoom");

	threadCheckBox = new QCheckBox(tr("multi threads"));
	threadCheckBox->setChecked(Qt::Checked);

	xcLock = new QToolButton(); xcLock->setCheckable(true);
	ycLock = new QToolButton(); ycLock->setCheckable(true);
	zcLock = new QToolButton(); zcLock->setCheckable(true);	
	
	// layout for mv control window
	layout->addWidget(cutLeftXSliderLabel, 0, 0, 1, 1);
	layout->addWidget(cutLeftXSlider, 0, 1, 1, 13);
	layout->addWidget(xcLock, 1, 0, 1, 1);
	layout->addWidget(cutRightXSlider, 1, 1, 1, 13);

	layout->addWidget(cutLeftYSliderLabel, 2, 0, 1, 1);
	layout->addWidget(cutLeftYSlider, 2, 1, 1, 13);
	layout->addWidget(ycLock, 3, 0, 1, 1);
	layout->addWidget(cutRightYSlider, 3, 1, 1, 13);

	layout->addWidget(cutLeftZSliderLabel, 4, 0, 1, 1);
	layout->addWidget(cutLeftZSlider, 4, 1, 1, 13);
	layout->addWidget(zcLock, 5, 0, 1, 1);
	layout->addWidget(cutRightZSlider, 5, 1, 1, 13);

	layout->addWidget(zoomSliderLabel, 6, 0, 1, 1);
	layout->addWidget(zoomSlider, 6, 1, 1, 13);

	layout->addWidget(threadCheckBox, 7, 0, 1, 14);

	// setup connections
	connect(cutLeftXSlider,    SIGNAL(valueChanged(int)), this, SLOT(onLeftXChanged(int)));
	connect(cutLeftYSlider,    SIGNAL(valueChanged(int)), this, SLOT(onLeftYChanged(int)));
	connect(cutLeftZSlider,    SIGNAL(valueChanged(int)), this, SLOT(onLeftZChanged(int)));
	connect(cutRightXSlider,    SIGNAL(valueChanged(int)), this, SLOT(onRightXChanged(int)));
	connect(cutRightYSlider,    SIGNAL(valueChanged(int)), this, SLOT(onRightYChanged(int)));
	connect(cutRightZSlider,    SIGNAL(valueChanged(int)), this, SLOT(onRightZChanged(int)));
	connect(zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(onZoomChanged(int)));
	connect(threadCheckBox, SIGNAL(toggled(bool)), this, SLOT(update()));

	if (0 && xcLock) {
		connect(xcLock, SIGNAL(toggled(bool)), this, SLOT(setXCutLock(bool)));
		connect(xcLock, SIGNAL(toggled(bool)), this, SLOT(setXCutLockIcon(bool))); setXCutLockIcon(false);
	}
	if (0 && ycLock) {
		connect(ycLock, SIGNAL(toggled(bool)), this, SLOT(setYCutLock(bool)));
		connect(ycLock, SIGNAL(toggled(bool)), this, SLOT(setYCutLockIcon(bool))); setYCutLockIcon(false);
	}
	if (0 && zcLock) {
		connect(zcLock, SIGNAL(toggled(bool)), this, SLOT(setZCutLock(bool)));
		connect(zcLock, SIGNAL(toggled(bool)), this, SLOT(setZCutLockIcon(bool))); setZCutLockIcon(false);
	}

	leftX = 0; rightX = 100;
	leftY = 0; rightY = 100;
	leftZ = 0; rightZ = 100;
	dxCut = 100; dyCut = 100; dzCut = 100;
	lockX = 0; lockY = 0; lockZ = 0;

	update_locked = false;
	update();
}

void MapViewWidget::update()
{
	if(update_locked) return;

	leftX = cutLeftXSlider->value();
	leftY = cutLeftYSlider->value(); 
	leftZ = cutLeftZSlider->value(); 
	rightX = cutRightXSlider->value(); 
	rightY = cutRightYSlider->value(); 
	rightZ = cutRightZSlider->value(); 
	zoom = zoomSlider->value();
	is_multi_thread = threadCheckBox->isChecked();

	paras.level = paras.level_num - zoom;
	V3DLONG in_sz0 = 0, in_sz1 = 0, in_sz2 = 0;
	mapview.getDownSamplingSize(paras.level, in_sz0, in_sz1, in_sz2);
	cout<<"paras.level = "<<paras.level<<endl;
	cout<<"in_sz0 = "<<in_sz0<<endl;
	cout<<"in_sz1 = "<<in_sz1<<endl;
	cout<<"in_sz2 = "<<in_sz2<<endl;
	paras.origin[0] = leftX/99.0 * in_sz0;
	paras.origin[1] = leftY/99.0 * in_sz1;
	paras.origin[2] = leftZ/99.0 * in_sz2;
	paras.outsz[0] = (rightX - leftX)/100.0 * in_sz0;
	paras.outsz[1] = (rightY - leftY)/100.0 * in_sz1;
	paras.outsz[2] = (rightZ - leftZ)/100.0 * in_sz2;

	// get curwin
	v3dhandleList winlist = callback->getImageWindowList();
	if(curwin == 0 || winlist.empty() || !winlist.contains(curwin)) curwin = callback->newImageWindow();

	// retrieve image from blocks
	unsigned char * outimg1d = 0;

	mapview.getImage(paras.level, outimg1d, paras.origin[0], paras.origin[1], paras.origin[2],
			paras.outsz[0], paras.outsz[1], paras.outsz[2], paras.is_use_thread);

	Image4DSimple * p4dimage = new Image4DSimple;

	p4dimage->setData(outimg1d, paras.outsz[0], paras.outsz[1], paras.outsz[2], paras.channel, V3D_UINT8); // todo : add more channel
	callback->setImage(curwin, p4dimage);
	callback->setImageName(curwin, QString(paras.hraw_dir.c_str()));
	callback->updateImageWindow(curwin);
}

void MapViewWidget::closeEvent(QCloseEvent *event)
{
	int ok = QMessageBox::information(0, "", "Are you sure to close?");
	if (ok) {
		event->accept();
	} else {
		event->ignore();
	}
}

void MapViewWidget::onLeftXChanged(int value)
{
	if(value < 0 || value > 99) return;
	leftX = value;
	if(lockX)
		cutRightXSlider->setValue(leftX + dxCut);
	else if(leftX >= rightX) cutRightXSlider->setValue(leftX+1);
	else update();
}

void MapViewWidget::onLeftYChanged(int value)
{
	if(value < 0 || value > 99) return;
	leftY = value;
	if(lockY)
		cutRightYSlider->setValue(leftY + dyCut);
	else if(leftY >= rightY) cutRightYSlider->setValue(leftY+1);
	else update();
}

void MapViewWidget::onLeftZChanged(int value)
{
	if(value < 0 || value > 99) return;
	leftZ = value;
	if(lockZ)
		cutRightZSlider->setValue(leftZ + dzCut);
	else if(leftZ >= rightZ) cutRightZSlider->setValue(leftZ+1);
	else update();
}

void MapViewWidget::onRightXChanged(int value)
{
	if(value < 1 || value > 100) return;
	rightX = value;
	if(lockX)
		cutLeftXSlider->setValue(leftX - dxCut);
	else if(rightX <= leftX) cutLeftXSlider->setValue(rightX-1);
	else update();
}

void MapViewWidget::onRightYChanged(int value)
{
	if(value < 1 || value > 100) return;
	rightY = value;
	if(lockY)
		cutLeftYSlider->setValue(leftY - dyCut);
	else if(rightY <= leftY) cutLeftYSlider->setValue(rightY-1);
	else update();
}

void MapViewWidget::onRightZChanged(int value)
{
	if(value < 1 || value > 100) return;
	rightZ = value;
	if(lockZ)
		cutLeftZSlider->setValue(leftZ - dzCut);
	else if(rightZ <= leftZ) cutLeftZSlider->setValue(rightZ-1);
	else update();
}

void MapViewWidget::onZoomChanged(int value)
{
	double midx = (leftX + rightX)/2.0;
	double lenx = (rightX - leftX)/pow(2.0, value-zoom);
	int leftX2 = midx - lenx/2.0 + 0.5;
	int rightX2 = leftX2 + (lenx + 0.5);

	double midy = (leftY + rightY)/2.0;
	double leny = (rightY - leftY)/pow(2.0, value-zoom);
	int leftY2 = midy - leny/2.0 + 0.5;
	int rightY2 = leftY2 + (leny + 0.5);

	double midz = (leftZ + rightZ)/2.0;
	double lenz = (rightZ - leftZ)/pow(2.0, value-zoom);
	int leftZ2 = midz - lenz/2.0 + 0.5;
	int rightZ2 = leftZ2 + (lenz + 0.5);

	leftX2 = MAX(leftX2, 0);
	rightX2 = MIN(rightX2, 100);
	leftY2 = MAX(leftY2, 0);
	rightY2 = MIN(rightY2, 100);
	leftZ2 = MAX(leftZ2, 0);
	rightZ2 = MIN(rightZ2, 100);

	zoom = value;

	update_locked = true;
	cutLeftXSlider->setValue(leftX2);
	cutLeftYSlider->setValue(leftY2);
	cutLeftZSlider->setValue(leftZ2);
	cutRightXSlider->setValue(rightX2);
	cutRightYSlider->setValue(rightY2);
	cutRightZSlider->setValue(rightZ2);
	update_locked = false;

	update();
}

void MapViewWidget::setXCutLock(bool b)
{       
    if (b)  dxCut = rightX - leftX;
    else    dxCut = 0;
    lockX = b? 1:0;  //110714
}       
void MapViewWidget::setYCutLock(bool b)
{   
    if (b)  dyCut = rightY - leftY;
    else    dyCut = 0;
    lockY = b? 1:0;  //110714
}   
void MapViewWidget::setZCutLock(bool b)
{   
    if (b)  dzCut = rightZ - leftZ;
    else    dzCut = 0;
    lockZ = b? 1:0;  //110714
}


void MapViewWidget::setXCutLockIcon(bool b)
{
    if (! xcLock)  return;
    if (b)
        xcLock->setIcon(QIcon(":/pic/Lockon.png"));
    else
        xcLock->setIcon(QIcon(":/pic/Lockoff.png"));
}
void MapViewWidget::setYCutLockIcon(bool b)
{
    if (! ycLock)  return;
    if (b)
        ycLock->setIcon(QIcon(":/pic/Lockon.png"));
    else
        ycLock->setIcon(QIcon(":/pic/Lockoff.png"));
}
void MapViewWidget::setZCutLockIcon(bool b)
{
    if (! zcLock)  return;
    if (b)
        zcLock->setIcon(QIcon(":/pic/Lockon.png"));
    else
        zcLock->setIcon(QIcon(":/pic/Lockoff.png"));
} 
