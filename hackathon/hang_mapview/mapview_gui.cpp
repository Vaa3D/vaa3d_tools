#include <iostream>
#include <QtGui>
#include <v3d_interface.h>
#include "mapview.h"
#include "mapview_gui.h"

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

	cutLeftXSlider = new QScrollBar(Qt::Horizontal);
	cutLeftXSlider->setRange(0, 100); 
	cutLeftXSlider->setSingleStep(1);
	cutLeftXSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	cutLeftXSlider->setValue(0);
	QLabel* cutLeftXSliderLabel = new QLabel("X-cut");

	cutRightXSlider = new QScrollBar(Qt::Horizontal);
	cutRightXSlider->setRange(0, 100); 
	cutRightXSlider->setSingleStep(1);
	cutRightXSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	cutRightXSlider->setValue(100);
	QLabel* cutRightXSliderLabel = new QLabel("");

	cutLeftYSlider = new QScrollBar(Qt::Horizontal);
	cutLeftYSlider->setRange(0, 100); 
	cutLeftYSlider->setSingleStep(1);
	cutLeftYSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	cutLeftYSlider->setValue(0);
	QLabel* cutLeftYSliderLabel = new QLabel("Y-cut");

	cutRightYSlider = new QScrollBar(Qt::Horizontal);
	cutRightYSlider->setRange(0, 100); 
	cutRightYSlider->setSingleStep(1);
	cutRightYSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	cutRightYSlider->setValue(100);
	QLabel* cutRightYSliderLabel = new QLabel("");

	cutLeftZSlider = new QScrollBar(Qt::Horizontal);
	cutLeftZSlider->setRange(0, 100); 
	cutLeftZSlider->setSingleStep(1);
	cutLeftZSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	cutLeftZSlider->setValue(0);
	QLabel* cutLeftZSliderLabel = new QLabel("Z-cut");

	cutRightZSlider = new QScrollBar(Qt::Horizontal);
	cutRightZSlider->setRange(0, 100); 
	cutRightZSlider->setSingleStep(1);
	cutRightZSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	cutRightZSlider->setValue(100);
	QLabel* cutRightZSliderLabel = new QLabel("");

	zoomSlider = new QScrollBar(Qt::Horizontal);
	zoomSlider->setRange(1, paras.level_num); 
	zoomSlider->setSingleStep(1);
	zoomSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	zoomSlider->setValue(paras.level_num - paras.level); // set minimum zoom, the thumbnail whole image would display
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
	connect(cutLeftXSlider,    SIGNAL(valueChanged(int)), this, SLOT(onValueChanged()));
	connect(cutLeftYSlider,    SIGNAL(valueChanged(int)), this, SLOT(onValueChanged()));
	connect(cutLeftZSlider,    SIGNAL(valueChanged(int)), this, SLOT(onValueChanged()));
	connect(cutRightXSlider,    SIGNAL(valueChanged(int)), this, SLOT(onValueChanged()));
	connect(cutRightYSlider,    SIGNAL(valueChanged(int)), this, SLOT(onValueChanged()));
	connect(cutRightZSlider,    SIGNAL(valueChanged(int)), this, SLOT(onValueChanged()));
	connect(zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged()));
	connect(threadCheckBox, SIGNAL(toggled(bool)), this, SLOT(onValueChanged()));

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
	paras.outsz[0] = (rightx - leftx)/100.0 * in_sz0;
	paras.outsz[1] = (righty - lefty)/100.0 * in_sz1;
	paras.outsz[2] = (rightz - leftz)/100.0 * in_sz2;
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

	updateTriView();
}
