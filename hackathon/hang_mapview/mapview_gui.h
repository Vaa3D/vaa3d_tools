#ifndef __MAPVIEW_GUI_H__
#define __MAPVIEW_GUI_H__

#include <iostream>
#include <QtGui>
#include <v3d_interface.h>
#include "mapview.h"

using namespace std;

struct Mapview_Paras {
	int L, M, N, l, m, n;//block
	int level_num;
	int channel;
	int level;           //current level
	V3DLONG outsz[4];    //output size
	V3DLONG origin[3];   //top-left corner pos
	QString hraw_dir; //prefix of files

	Mapview_Paras()
	{
		hraw_dir=QString("");
		outsz[0]=outsz[1]=outsz[2]=outsz[3]=0;
		origin[0]=origin[1]=origin[2]=0;
		L = M = N = l = m = n = 0;
		level_num = 0;
		level=0;
	}
};

class MapViewWidget : public QWidget
{
	Q_OBJECT;

public:
	MapViewWidget(V3DPluginCallback2 * _callback, Mapview_Paras _mapview_paras,  QWidget *parent = 0) : QWidget(parent)
	{
		callback = _callback; curwin = 0; mapview_paras = _mapview_paras;

		setWindowTitle("Mapview Control");
        /*setWindowFlags( Qt::Widget
                | Qt::Tool
                | Qt::CustomizeWindowHint | Qt::WindowTitleHint  //only title bar, disable buttons on title bar
                );
		*/
        setFixedWidth(350);
        setFixedHeight(150);

        QGridLayout *layout = new QGridLayout(this);

        // get X Y Z size
        V3DLONG ts0, ts1, ts2; // block nums
        V3DLONG bs0, bs1, bs2; // block size
        V3DLONG dimx, dimy, dimz;
		mapview.setPara(mapview_paras.hraw_dir.toStdString(), mapview_paras.L, mapview_paras.M, mapview_paras.N, mapview_paras.l, mapview_paras.m, mapview_paras.n, mapview_paras.channel);
        mapview.getBlockTillingSize(mapview_paras.level, ts0, ts1, ts2, bs0, bs1, bs2);
        dimx = ts0*bs0; dimy = ts1*bs1; dimz = ts2*bs2;
		cout<<"dimx = "<<dimx<<" dimy = "<<dimy<<" dimz = "<<dimz<<endl;
		cout<<"outsz[0] = "<<mapview_paras.outsz[0]<<" outsz[1] = "<<mapview_paras.outsz[1]<<" outsz[2] = "<<mapview_paras.outsz[2]<<endl;

        // zoom range
        int dim_zoom= mapview_paras.level_num;

        xSlider_mapv = new QScrollBar(Qt::Horizontal);
        xSlider_mapv->setRange(0, dimx-mapview_paras.outsz[0]); //need redefine range
		xSlider_mapv->setSingleStep(1);
        xSlider_mapv->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        QLabel* xSliderLabel_mapv = new QLabel("X");

        xValueSpinBox_mapv = new QSpinBox;
        xValueSpinBox_mapv->setRange(0, dimx-mapview_paras.outsz[0]);
        xValueSpinBox_mapv->setSingleStep(1);
        xValueSpinBox_mapv->setValue(mapview_paras.origin[0]);

        ySlider_mapv = new QScrollBar(Qt::Horizontal);
        ySlider_mapv->setRange(0, dimy-mapview_paras.outsz[1]); //need redefine range
		ySlider_mapv->setSingleStep(1);
        ySlider_mapv->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        QLabel* ySliderLabel_mapv = new QLabel("Y");

        yValueSpinBox_mapv = new QSpinBox;
        yValueSpinBox_mapv->setRange(0, dimy-mapview_paras.outsz[1]);
        yValueSpinBox_mapv->setSingleStep(1);
        yValueSpinBox_mapv->setValue(mapview_paras.origin[1]);

        zSlider_mapv = new QScrollBar(Qt::Horizontal);
        zSlider_mapv->setRange(0, dimz-mapview_paras.outsz[2]); //need redefine range
		zSlider_mapv->setSingleStep(1);
		zSlider_mapv->setPageStep(1);
        zSlider_mapv->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        QLabel* zSliderLabel_mapv = new QLabel("Z");

        zValueSpinBox_mapv = new QSpinBox;
        zValueSpinBox_mapv->setRange(0, dimz-mapview_paras.outsz[2]);
        zValueSpinBox_mapv->setSingleStep(1);
        zValueSpinBox_mapv->setValue(mapview_paras.origin[2]);

        // zoom slider
        zoomSlider_mapv = new QScrollBar(Qt::Horizontal);
        zoomSlider_mapv->setRange(0, dim_zoom-1); //need redefine range
		zoomSlider_mapv->setSingleStep(1);
        zoomSlider_mapv->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        QLabel* zoomLabel_mapv = new QLabel("Zoom");

        zoomSpinBox_mapv = new QSpinBox;
        zoomSpinBox_mapv->setRange(0, dim_zoom-1);
        zoomSpinBox_mapv->setSingleStep(1);
        zoomSpinBox_mapv->setValue(mapview_paras.level);

        // layout for mv control window
        layout->addWidget(zSliderLabel_mapv, 0, 0, 1, 1);
        layout->addWidget(zSlider_mapv, 0, 1, 1, 13);
        layout->addWidget(zValueSpinBox_mapv, 0, 14, 1, 6);

        layout->addWidget(xSliderLabel_mapv, 1, 0, 1, 1);
        layout->addWidget(xSlider_mapv, 1, 1, 1, 13);
        layout->addWidget(xValueSpinBox_mapv, 1, 14, 1, 6);

        layout->addWidget(ySliderLabel_mapv, 2, 0, 1, 1);
        layout->addWidget(ySlider_mapv, 2, 1, 1, 13);
        layout->addWidget(yValueSpinBox_mapv, 2, 14, 1, 6);

        layout->addWidget(zoomLabel_mapv, 3, 0, 1, 1);
        layout->addWidget(zoomSlider_mapv, 3, 1, 1, 13);
        layout->addWidget(zoomSpinBox_mapv, 3, 14, 1, 6);

        // setup connections
		connect(xSlider_mapv,    SIGNAL(valueChanged(int)), this, SLOT(changeXOffset_mapv(int)));
		connect(ySlider_mapv,    SIGNAL(valueChanged(int)), this, SLOT(changeYOffset_mapv(int)));
		connect(zSlider_mapv,    SIGNAL(valueChanged(int)), this, SLOT(changeZOffset_mapv(int)));
        connect(zoomSlider_mapv, SIGNAL(valueChanged(int)), this, SLOT(changeLevel_mapv(int)));

        //connect(xValueSpinBox_mapv,    SIGNAL(valueChanged(int)), this, SLOT(changeXOffset_mapv(int)));
        //connect(yValueSpinBox_mapv,    SIGNAL(valueChanged(int)), this, SLOT(changeYOffset_mapv(int)));
        //connect(zValueSpinBox_mapv,    SIGNAL(valueChanged(int)), this, SLOT(changeZOffset_mapv(int)));
        //connect(zoomSpinBox_mapv,      SIGNAL(valueChanged(int)), this, SLOT(changeLevel_mapv(int)));

		updateTriView();
	}

	~MapViewWidget(){}
	void updateLevels(int level)
	{
        // get X Y Z size
        V3DLONG ts0, ts1, ts2; // block nums
        V3DLONG bs0, bs1, bs2; // block size
        V3DLONG dimx, dimy, dimz;
        mapview.getBlockTillingSize(level, ts0, ts1, ts2, bs0, bs1, bs2);
        dimx = ts0*bs0;   dimy = ts1*bs1;   dimz = ts2*bs2;

		V3DLONG orig0 = mapview_paras.origin[0] / pow(2.0, level - mapview_paras.level);
		V3DLONG orig1 = mapview_paras.origin[1] / pow(2.0, level - mapview_paras.level);
		V3DLONG orig2 = mapview_paras.origin[2] / pow(2.0, level - mapview_paras.level);
		V3DLONG max_range0 = dimx - mapview_paras.outsz[0];
		V3DLONG max_range1 = dimy - mapview_paras.outsz[1];
		V3DLONG max_range2 = dimz - mapview_paras.outsz[2];
		cout<<"level = "<<level<<endl;
		cout<<"outsz[0] = "<<mapview_paras.outsz[0]<<" dimx = "<<dimx<<" max_range0 = "<<max_range0<<endl;
		cout<<"outsz[1] = "<<mapview_paras.outsz[1]<<" dimy = "<<dimy<<" max_range1 = "<<max_range1<<endl;
		cout<<"outsz[2] = "<<mapview_paras.outsz[2]<<" dimz = "<<dimz<<" max_range2 = "<<max_range2<<endl;
		mapview_paras.origin[0] = MIN(orig0, max_range0);
		mapview_paras.origin[1] = MIN(orig0, max_range1);
		mapview_paras.origin[2] = MIN(orig0, max_range2);

		mapview_paras.level = level;

        xSlider_mapv->setRange(0, max_range0);
        xValueSpinBox_mapv->setRange(0, max_range0);
        xValueSpinBox_mapv->setValue(mapview_paras.origin[0]);

        ySlider_mapv->setRange(0, max_range1);
        yValueSpinBox_mapv->setRange(0, max_range1);
        yValueSpinBox_mapv->setValue(mapview_paras.origin[1]);

        zSlider_mapv->setRange(0, max_range2);
        zValueSpinBox_mapv->setRange(0, max_range2);
        zValueSpinBox_mapv->setValue(mapview_paras.origin[2]);
	}
	void updateTriView()
	{
		// get curwin
		v3dhandleList winlist = callback->getImageWindowList();
		if(curwin == 0 || winlist.empty() || !winlist.contains(curwin)) curwin = callback->newImageWindow();

		// retrieve image from blocks
		unsigned char * outimg1d = 0;

		mapview.getImage(mapview_paras.level, outimg1d, mapview_paras.origin[0], mapview_paras.origin[1], mapview_paras.origin[2],
				mapview_paras.outsz[0], mapview_paras.outsz[1], mapview_paras.outsz[2]);

		Image4DSimple * p4dimage = new Image4DSimple;

		p4dimage->setData(outimg1d, mapview_paras.outsz[0], mapview_paras.outsz[1], mapview_paras.outsz[2], mapview_paras.channel, V3D_UINT8); // todo : add more channel
		callback->setImage(curwin, p4dimage);
		callback->setImageName(curwin, mapview_paras.hraw_dir);
		callback->updateImageWindow(curwin);
	}

private:
	ImageMapView mapview;
	Mapview_Paras mapview_paras;
	V3DPluginCallback2 * callback;
	v3dhandle curwin;

	QScrollBar *xSlider_mapv, *ySlider_mapv, *zSlider_mapv;
	QSpinBox *xValueSpinBox_mapv, *yValueSpinBox_mapv, *zValueSpinBox_mapv, *zoomSpinBox_mapv;
	QScrollBar *zoomSlider_mapv;

	void closeEvent(QCloseEvent *event)
	{
		int ok = QMessageBox::warning(0, "", "Are you sure to close?");
		if (ok) {
			event->accept();
		} else {
			event->ignore();
		}
	}

public slots:
	void changeXOffset_mapv(int x)
	{
		if(xSlider_mapv->value() != x) xSlider_mapv->setValue(x);
		else if(xValueSpinBox_mapv->value() != x) xValueSpinBox_mapv->setValue(x);
		{
			mapview_paras.origin[0] = x;
			updateTriView();
		}
	}
	void changeYOffset_mapv(int y)
	{
		if(ySlider_mapv->value() != y) ySlider_mapv->setValue(y);
		else if(yValueSpinBox_mapv->value() != y) yValueSpinBox_mapv->setValue(y);
		{
			mapview_paras.origin[1] = y;
			updateTriView();
		}
	}
	void changeZOffset_mapv(int z)
	{
		if(zSlider_mapv->value() != z) zSlider_mapv->setValue(z);
		else if(zValueSpinBox_mapv->value() != z) zValueSpinBox_mapv->setValue(z);
		{
			mapview_paras.origin[2] = z;
			updateTriView();
		}
	}
	void changeLevel_mapv(int level)
	{
		if(zoomSlider_mapv->value() != level) zoomSlider_mapv->setValue(level);
		else if(zoomSpinBox_mapv->value() != level) zoomSpinBox_mapv->setValue(level);
		{
			mapview_paras.level = level;
			updateLevels(level);
			updateTriView();
		}
	}
};

#endif 
