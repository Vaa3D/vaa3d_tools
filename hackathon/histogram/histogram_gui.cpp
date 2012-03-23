//histogram_gui.cpp
//adapted from this file:
/*
 * barFigureDialog.cpp
 *
 *  Created on: May 1, 2009
 *      Author: ruanzongcai
 */
//2012-03-23: customize the histogram for display image histogram

#include "histogram_gui.h"
#include "volimg_proc.h"


histogramDialog::histogramDialog(QVector< QVector<int> >& vvec, QStringList labelsOfLeftTop, QString labelOfRightBottom,
		QWidget *parent, QSize figSize, QColor barColor)
: QDialog(parent)
{
	setWindowFlags(Qt::Popup /*| Qt::WindowStaysOnTopHint*/ | Qt::Tool); setAttribute(Qt::WA_MacAlwaysShowToolWindow);

	QGridLayout *layout = new QGridLayout;

	int nChannel = vvec.size();
	QLabel ** labelPicture = new QLabel * [nChannel]; //revised by PHC, 2010-05-20
	int row = 1;
	for (int i=0; i<nChannel; i++)
	{
		labelPicture[i] = new QLabel;
		labelPicture[i]->setFrameStyle(QFrame::Box | QFrame::Plain);	labelPicture[i]->setLineWidth(1);

		labelPicture[i]->resize(figSize);
		QVector<int> & vec = vvec[i];
		
		V3DLONG imin, imax;
		int vmin, vmax;
		minMaxInVector(vec.data(), vec.size(), imin, vmin, imax, vmax);
		QString desc = QString("max=%1").arg(vmax);

		QVector<int> vec_rescaled = vvec[i];
		for (V3DLONG j=0;j<vec.size();j++)
			vec_rescaled[j] = vec[j]*500.0/vmax;
		QImage fig = drawBarFigure( vec_rescaled, barColor);


		layout->addWidget(new QLabel(labelsOfLeftTop[i]), row, 0);
		layout->addWidget(new QLabel(desc),               row++, 0, Qt::AlignRight);

		layout->addWidget( labelPicture[i],  row++, 0);

		QPicture pic;
		QPainter p;
		p.begin( &pic );
			p.drawImage( labelPicture[i]->rect(), fig );
		p.end();
		labelPicture[i]->setPicture( pic );
	}
	layout->addWidget(new QLabel(QString("%1").arg(0)),  row, 0);
	layout->addWidget(new QLabel(labelOfRightBottom),    row++, 0, Qt::AlignRight);
	layout->setSizeConstraint(QLayout::SetFixedSize); //same as dlg->setFixedSize(dlg->size()); // fixed layout after show

	setLayout(layout);
	if (labelPicture) {delete []labelPicture; labelPicture=0;} //added by PHC, 2010-05-20
}

histogramDialog::~histogramDialog()
{
	// TODO Auto-generated destructor stub
}

void histogramDialog::closeEvent(QCloseEvent* e)
{
	this->deleteLater();
}


QImage drawBarFigure(QVector<int>& vec, QColor barColor)
{
	//find bound
	int x1,x2, y1,y2;
	x1=x2= 0;
	y1=y2= 0;
	for (int i=0; i<vec.size(); i++)
	{
		int value = vec[i];
		if (value > y2) y2 = value;
	}
	x2 = vec.size()-1;

	QSize size(x2-x1+1, y2-y1+1);
	QImage img(size, QImage::Format_RGB32);

	//paint bar figure
	QPainter p(&img);
	QRect rect(0, 0, size.width(), size.height());
	QBrush shade(QColor(255,255,255));
	p.fillRect(rect, shade);
	for (int i=0; i<vec.size(); i++)
	{
		int value = vec[i];
		QRect rect(i, 0, 1, value);
		QBrush shade( barColor );
		p.fillRect(rect, shade);
	}

	return img.mirrored(); // flip y-direction
}


