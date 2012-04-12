#include <iostream>
#include <QtGui>
#include <v3d_interface.h>
#include "mapview.h"
#include "gui.h"

using namespace std;

MapViewWidget::MapViewWidget(V3DPluginCallback2 * _callback, Mapview_Paras _paras,  QWidget *parent) : QWidget(parent)
{
	callback = _callback; curwin = 0; paras = _paras;
	V3DLONG L = paras.L;
	V3DLONG M = paras.M;
	V3DLONG N = paras.N;
	V3DLONG l = paras.l;
	V3DLONG m = paras.m;
	V3DLONG n = paras.n;

	setWindowTitle("Mapview Control");
	/*setWindowFlags( Qt::Widget
	  | Qt::Tool
	  | Qt::CustomizeWindowHint | Qt::WindowTitleHint  //only title bar, disable buttons on title bar
	  );
	  */
	setFixedWidth(350);
	setFixedHeight(300);

	QGridLayout *layout = new QGridLayout(this);

	// get X Y Z size
	V3DLONG ts0, ts1, ts2; // block nums
	V3DLONG bs0, bs1, bs2; // block size
	V3DLONG dimx, dimy, dimz;
	mapview.setPara(paras.hraw_dir.toStdString(), paras.L, paras.M, paras.N, paras.l, paras.m, paras.n, paras.channel);
	mapview.getBlockTillingSize(paras.level, ts0, ts1, ts2, bs0, bs1, bs2);
	dimx = ts0*bs0; dimy = ts1*bs1; dimz = ts2*bs2;
	cout<<"dimx = "<<dimx<<" dimy = "<<dimy<<" dimz = "<<dimz<<endl;
	cout<<"outsz[0] = "<<paras.outsz[0]<<" outsz[1] = "<<paras.outsz[1]<<" outsz[2] = "<<paras.outsz[2]<<endl;

	// zoom range
	int dim_zoom= paras.level_num;

	cutLeftXSlider_mapv = new QScrollBar(Qt::Horizontal);
	cutLeftXSlider_mapv->setRange(0, 100); 
	cutLeftXSlider_mapv->setSingleStep(1);
	cutLeftXSlider_mapv->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	QLabel* cutLeftXSliderLabel_mapv = new QLabel("X-cut");

	cutRightXSlider_mapv = new QScrollBar(Qt::Horizontal);
	cutRightXSlider_mapv->setRange(0, 100); 
	cutRightXSlider_mapv->setSingleStep(1);
	cutRightXSlider_mapv->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	QLabel* cutRightXSliderLabel_mapv = new QLabel("");

	cutLeftYSlider_mapv = new QScrollBar(Qt::Horizontal);
	cutLeftYSlider_mapv->setRange(0, 100); 
	cutLeftYSlider_mapv->setSingleStep(1);
	cutLeftYSlider_mapv->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	QLabel* cutLeftYSliderLabel_mapv = new QLabel("Y-cut");

	cutRightYSlider_mapv = new QScrollBar(Qt::Horizontal);
	cutRightYSlider_mapv->setRange(0, 100); 
	cutRightYSlider_mapv->setSingleStep(1);
	cutRightYSlider_mapv->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	QLabel* cutRightYSliderLabel_mapv = new QLabel("");

	cutLeftZSlider_mapv = new QScrollBar(Qt::Horizontal);
	cutLeftZSlider_mapv->setRange(0, 100); 
	cutLeftZSlider_mapv->setSingleStep(1);
	cutLeftZSlider_mapv->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	QLabel* cutLeftZSliderLabel_mapv = new QLabel("Z-cut");

	cutRightZSlider_mapv = new QScrollBar(Qt::Horizontal);
	cutRightZSlider_mapv->setRange(0, 100); 
	cutRightZSlider_mapv->setSingleStep(1);
	cutRightZSlider_mapv->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	QLabel* cutRightZSliderLabel_mapv = new QLabel("");

	zoomSlider_mapv = new QScrollBar(Qt::Horizontal);
	zoomSlider_mapv->setRange(0, 100); 
	zoomSlider_mapv->setSingleStep(1);
	zoomSlider_mapv->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	QLabel* zoomSliderLabel_mapv = new QLabel("zoom");

	threadCheckBox = new QCheckBox(tr("multi threads"));
	threadCheckBox->setChecked(Qt::Checked);
	
	// layout for mv control window
	layout->addWidget(cutLeftXSliderLabel_mapv, 0, 0, 1, 1);
	layout->addWidget(cutLeftXSlider_mapv, 0, 1, 1, 13);
	layout->addWidget(cutRightXSliderLabel_mapv, 1, 0, 1, 1);
	layout->addWidget(cutRightXSlider_mapv, 1, 1, 1, 13);

	layout->addWidget(cutLeftYSliderLabel_mapv, 2, 0, 1, 1);
	layout->addWidget(cutLeftYSlider_mapv, 2, 1, 1, 13);
	layout->addWidget(cutRightYSliderLabel_mapv, 3, 0, 1, 1);
	layout->addWidget(cutRightYSlider_mapv, 3, 1, 1, 13);

	layout->addWidget(cutLeftZSliderLabel_mapv, 4, 0, 1, 1);
	layout->addWidget(cutLeftZSlider_mapv, 4, 1, 1, 13);
	layout->addWidget(cutRightZSliderLabel_mapv, 5, 0, 1, 1);
	layout->addWidget(cutRightZSlider_mapv, 5, 1, 1, 13);

	layout->addWidget(zoomSliderLabel_mapv, 6, 0, 1, 1);
	layout->addWidget(zoomSlider_mapv, 6, 1, 1, 13);

	layout->addWidget(threadCheckBox, 7, 0, 1, 14);

	// setup connections
	connect(cutLeftXSlider_mapv,    SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)));
	connect(cutLeftYSlider_mapv,    SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)));
	connect(cutLeftZSlider_mapv,    SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)));
	connect(cutRightXSlider_mapv,    SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)));
	connect(cutRightYSlider_mapv,    SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)));
	connect(cutRightZSlider_mapv,    SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)));
	connect(zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)));
	connect(threadCheckBox, SIGNAL(toggled(bool)), this, SLOT(setMultiThreads(bool)));
	updateTriView();
}

void MapViewWidget::updateTriView()
{
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
	callback->setImageName(curwin, paras.hraw_dir);
	callback->updateImageWindow(curwin);
}

void MapViewWidget::closeEvent(QCloseEvent *event)
{
	int ok = QMessageBox::warning(0, "", "Are you sure to close?");
	if (ok) {
		event->accept();
	} else {
		event->ignore();
	}
}

void MapViewWidget::onValueChanged()
{
	void * button = sender();
	V3DLONG sz0 = paras.outsz[0];
	V3DLONG sz1 = paras.outsz[1];
	V3DLONG sz2 = paras.outsz[2];
	V3DLONG L = paras.L;
	V3DLONG M = paras.M;
	V3DLONG N = paras.N;
	V3DLONG l = paras.l;
	V3DLONG m = paras.m;
	V3DLONG n = paras.n;
	if(button == cutLeftXSlider) leftx = cutLeftXSlider->value();
	else if(button == cutLeftYSlider) lefty = cutLeftYSlider->value();
	else if(button == cutLeftZSlider) leftz = cutLeftZSlider->value();
	else if(button == cutRightXSlider) rightx = cutRightXSlider->value();
	else if(button == cutRightYSlider) righty = cutRightYSlider->value();
	else if(button == cutRightZSlider) rightz = cutRightZSlider->value();
	else if(button == zoomSlider) zoom = zoomSlider->value();
	else if(button == threadCheckBox) is_multi_thread = threadCheckBox->isChecked();

	int level = paras.level_num - zoom0;

	updateTriView();
}
