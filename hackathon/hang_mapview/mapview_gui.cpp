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
	
	// layout for mv control window
	layout->addWidget(cutLeftXSliderLabel, 0, 0, 1, 1);
	layout->addWidget(cutLeftXSlider, 0, 1, 1, 13);
	layout->addWidget(cutRightXSliderLabel, 1, 0, 1, 1);
	layout->addWidget(cutRightXSlider, 1, 1, 1, 13);

	layout->addWidget(cutLeftYSliderLabel, 2, 0, 1, 1);
	layout->addWidget(cutLeftYSlider, 2, 1, 1, 13);
	layout->addWidget(cutRightYSliderLabel, 3, 0, 1, 1);
	layout->addWidget(cutRightYSlider, 3, 1, 1, 13);

	layout->addWidget(cutLeftZSliderLabel, 4, 0, 1, 1);
	layout->addWidget(cutLeftZSlider, 4, 1, 1, 13);
	layout->addWidget(cutRightZSliderLabel, 5, 0, 1, 1);
	layout->addWidget(cutRightZSlider, 5, 1, 1, 13);

	layout->addWidget(zoomSliderLabel, 6, 0, 1, 1);
	layout->addWidget(zoomSlider, 6, 1, 1, 13);

	layout->addWidget(threadCheckBox, 7, 0, 1, 14);

	// setup connections
	connect(cutLeftXSlider,    SIGNAL(valueChanged(int)), this, SLOT(update()));
	connect(cutLeftYSlider,    SIGNAL(valueChanged(int)), this, SLOT(update()));
	connect(cutLeftZSlider,    SIGNAL(valueChanged(int)), this, SLOT(update()));
	connect(cutRightXSlider,    SIGNAL(valueChanged(int)), this, SLOT(update()));
	connect(cutRightYSlider,    SIGNAL(valueChanged(int)), this, SLOT(update()));
	connect(cutRightZSlider,    SIGNAL(valueChanged(int)), this, SLOT(update()));
	connect(zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(update()));
	connect(threadCheckBox, SIGNAL(toggled(bool)), this, SLOT(update()));

	update();
	updateTriView();
}

void MapViewWidget::updateTriView()
{
	paras.level = paras.level_num - zoom;
	V3DLONG ts0, ts1, ts2, bs0, bs1, bs2;
	mapview.getBlockTillingSize(paras.level, ts0, ts1, ts2, bs0, bs1, bs2);
	V3DLONG in_sz0 = ts0 * bs0;
	V3DLONG in_sz1 = ts1 * bs1;
	V3DLONG in_sz2 = ts2 * bs2;
	cout<<"paras.level = "<<paras.level<<endl;
	cout<<"in_sz0 = "<<in_sz0<<endl;
	cout<<"in_sz1 = "<<in_sz1<<endl;
	cout<<"in_sz2 = "<<in_sz2<<endl;
	paras.origin[0] = leftx/99.0 * in_sz0;
	paras.origin[1] = lefty/99.0 * in_sz1;
	paras.origin[2] = leftz/99.0 * in_sz2;
	paras.outsz[0] = (rightx - leftx)/100.0 * in_sz0;
	paras.outsz[1] = (righty - lefty)/100.0 * in_sz1;
	paras.outsz[2] = (rightz - leftz)/100.0 * in_sz2;

	// get curwin
	v3dhandleList winlist = callback->getImageWindowList();
	if(curwin == 0 || winlist.empty() || !winlist.contains(curwin)) curwin = callback->newImageWindow();

	// retrieve image from blocks
	unsigned char * outimg1d = 0;

	cout<<"paras.level = "<<paras.level<<endl;
	cout<<"paras.origin[0] = "<<paras.origin[0]<<endl;
	cout<<"paras.origin[1] = "<<paras.origin[1]<<endl;
	cout<<"paras.origin[2] = "<<paras.origin[2]<<endl;
	cout<<"paras.outsz[0] = "<<paras.outsz[0]<<endl;
	cout<<"paras.outsz[1] = "<<paras.outsz[1]<<endl;
	cout<<"paras.outsz[2] = "<<paras.outsz[2]<<endl;
	cout<<"is_use_thread = "<<paras.is_use_thread<<endl;
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

void MapViewWidget::update()
{
	leftx = cutLeftXSlider->value();
	lefty = cutLeftYSlider->value(); 
	leftz = cutLeftZSlider->value(); 
	rightx = cutRightXSlider->value(); 
	righty = cutRightYSlider->value(); 
	rightz = cutRightZSlider->value(); 
	zoom = zoomSlider->value();
	is_multi_thread = threadCheckBox->isChecked();

	if(leftx >= rightx){cutRightXSlider->setValue(leftx+1); return;}
	if(lefty >= righty){cutRightYSlider->setValue(lefty+1); return;}
	if(leftz >= rightz){cutRightZSlider->setValue(leftz+1); return;}
	if(rightx <= leftx){cutLeftXSlider->setValue(rightx-1); return;}
	if(righty <= lefty){cutLeftYSlider->setValue(righty-1); return;}
	if(rightz <= leftz){cutLeftZSlider->setValue(rightz-1); return;}

	updateTriView();
}
