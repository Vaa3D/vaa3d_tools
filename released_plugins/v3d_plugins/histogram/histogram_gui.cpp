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
#include <math.h>


histogramDialog::histogramDialog(QVector< QVector<int> >& vvec, QStringList labelsOfLeftTop, QString labelOfRightBottom,
		QWidget *parent, QSize figSize, QColor barColor)
: QDialog(parent)

{
	setWindowFlags(Qt::Popup /*| Qt::WindowStaysOnTopHint*/ | Qt::Tool); setAttribute(Qt::WA_MacAlwaysShowToolWindow);

	QGridLayout *layout = new QGridLayout;

	nChannel = vvec.size();
	data = vvec;
	barColor = barColor;
	figSize = figSize;
	labelPicture = new QLabel * [nChannel]; //revised by PHC, 2010-05-20
	int row = 1;
	for (int i=0; i<nChannel; i++)
	{
		labelPicture[i] = new QLabel;
		labelPicture[i]->setFrameStyle(QFrame::Box | QFrame::Plain);	labelPicture[i]->setLineWidth(1);

		labelPicture[i]->resize(figSize);

		QVector<int> & vec = data[i];
		
		V3DLONG imin, imax;
		int vmin, vmax;
		minMaxInVector(vec.data(), vec.size(), imin, vmin, imax, vmax);
		QString desc = QString("max=%1").arg(vmax);
		
		layout->addWidget(new QLabel(labelsOfLeftTop[i]), row, 0);
		layout->addWidget(new QLabel(desc),               row++, 0, Qt::AlignRight);

		layout->addWidget( labelPicture[i],  row++, 0);

	}
	layout->addWidget(new QLabel(QString("%1").arg(0)),  row, 0);
	layout->addWidget(new QLabel(labelOfRightBottom),    row++, 0, Qt::AlignRight);
	//add option button to display histogram normal or log10
	normalButton = new QPushButton("Display normal histogram");
	logButton = new QPushButton("Display log 10 histogram");
	connect(normalButton, SIGNAL(clicked()), this, SLOT(updateBar()));
	connect(logButton, SIGNAL(clicked()), this, SLOT(updateBar()));
	layout->addWidget(normalButton, row, 0, Qt::AlignLeft);
	layout->addWidget(logButton, row++, 0, Qt::AlignRight);

	layout->setSizeConstraint(QLayout::SetFixedSize); //same as dlg->setFixedSize(dlg->size()); // fixed layout after show

	setLayout(layout);
	//if (labelPicture) {delete []labelPicture; labelPicture=0;} //added by PHC, 2010-05-20

	updateBar();
}

/*histogramDialog::~histogramDialog()
{
	// TODO Auto-generated destructor stub
}
void histogramDialog::closeEvent(QCloseEvent* e)
{
	this->deleteLater();
}
*/

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

void histogramDialog::updateBar()
{
	QPushButton* button = (QPushButton*)sender();
	for (int i=0;i<nChannel;i++)
	{
		V3DLONG imin, imax;
		int vmin, vmax;
		QVector<int> vec_rescaled = data[i];
		if (button==logButton)
		{
			for (int j=0;j<vec_rescaled.size();j++)
			{
				if (vec_rescaled[j]!=0)
					vec_rescaled[j] = log10(vec_rescaled[j])* 1000.0;
			}
		}
		minMaxInVector(vec_rescaled.data(), vec_rescaled.size(), imin, vmin, imax, vmax);
		for (V3DLONG j=0;j<vec_rescaled.size();j++)
			vec_rescaled[j] = vec_rescaled[j]*500.0/vmax;
	
		QImage fig = drawBarFigure( vec_rescaled, barColor);
		QPicture pic;
		QPainter p;
		p.begin( &pic );
			QRect rectBar = labelPicture[i]->rect();
			p.drawImage( QRect(labelPicture[i]->rect().topLeft(),QSize(rectBar.width()-2, rectBar.height()-2) ), fig );
		p.end();
		labelPicture[i]->setPicture( pic );
	}

}
