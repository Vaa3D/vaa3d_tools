#ifndef __HISTOGRAM_GUI_H__
#define __HISTOGRAM_GUI_H__

#include <string>
#include <QtGui>
#include <vector>
#include <v3d_interface.h>

#include "color_xyz.h"

using namespace std;

struct DataChannelColor
{
	int n;			  // index
	RGBA8 color;
	bool on;
};


class HistogramDialog : public QDialog
{
	Q_OBJECT

public:
	HistogramDialog(QWidget * parent);

	~HistogramDialog(){}

	void paintEvent(QPaintEvent *event);
	void paint(QPainter *p);

public slots:
	void accept()
	{
		return QDialog::accept();
	}

	void reject()
	{
		return QDialog::reject();
	}

public:
	QList <DataChannelColor> listChannels;
     int disp_width, disp_height;
     int histscale;
     vector<V3DLONG> hist;

	QLabel * channelLabel;
	QFrame * channelFrame;
	QLabel * histLabel;
	QFrame * histFrame;

	QPushButton * ok;
	QPushButton * cancel;

	QGridLayout * gridLayout;
};

#endif
