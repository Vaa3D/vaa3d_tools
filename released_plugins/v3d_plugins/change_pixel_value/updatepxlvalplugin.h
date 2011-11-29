/* updatepxlvalplugin.h
 * 2009-08-21: create this program by Yang Yu
 */


#ifndef __UPDATEPXLVAL_H__
#define __UPDATEPXLVAL_H__

//   Set a pixel at your defined postion
//

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

class UpdatePxlValPlugin : public QObject, public V3DSingleImageInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DSingleImageInterface2_1)

public:
	UpdatePxlValPlugin() {}
    QStringList menulist() const;
    void processImage(const QString &arg, Image4DSimple *p4DImage, QWidget *parent);
    float getPluginVersion() const {return 1.1f;}
};

class ChangePxlValDialog : public QDialog
{
    Q_OBJECT
	
public:
    ChangePxlValDialog(Image4DSimple *p4DImage, QWidget *parent)
	{
		if (! p4DImage) return;
		
		printf("Passing data to data1d\n");
		pagesz = p4DImage->getTotalUnitNumberPerChannel();
		
		N = p4DImage->getXDim();
		M = p4DImage->getYDim();
		P = p4DImage->getZDim();
		sc = p4DImage->getCDim();
		
		printf("dimx %ld dimy %ld dimz %ld dimchannel %ld \n", N, M, P, sc);
		
		data1d = p4DImage->getRawData();	

		nx=0; ny=0; nz=0;
		
		//create a dialog
		coord_x = new QSpinBox();
		coord_y = new QSpinBox();
		coord_z = new QSpinBox();
		
		coord_x->setMaximum(N); coord_x->setMinimum(1); coord_x->setValue(1);
		coord_y->setMaximum(M); coord_y->setMinimum(1); coord_y->setValue(1);
		coord_z->setMaximum(P); coord_z->setMinimum(1); coord_z->setValue(1);
		
		nlabel = new QLabel *[sc];
		for(V3DLONG c=0; c<sc; c++)
		{
			nlabel[c] = new QLabel(QObject::tr("Set new intensity value at channel %1: ").arg(c+1));
		}
		
		nval = new QSpinBox *[sc];
		for(V3DLONG c=0; c<sc; c++)
		{
			nval[c] = new QSpinBox();
		}
		
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		
		gridLayout = new QGridLayout();
		
		labelx = new QLabel(QObject::tr("image x: "));
		labely = new QLabel(QObject::tr("image y: "));
		labelz = new QLabel(QObject::tr("image z: "));
		
		gridLayout->addWidget(labelx, 0,0); gridLayout->addWidget(coord_x, 0,1);
		gridLayout->addWidget(labely, 1,0); gridLayout->addWidget(coord_y, 1,1);
		gridLayout->addWidget(labelz, 2,0); gridLayout->addWidget(coord_z, 2,1);
		
		for(V3DLONG c=0; c<sc; c++)
		{
			gridLayout->addWidget(nlabel[c], c+3,0);
			gridLayout->addWidget(nval[c], c+3,1);
			
			nval[c]->setMaximum(255);
			nval[c]->setMinimum(0);
		}
		
		gridLayout->addWidget(cancel, sc+3,0); gridLayout->addWidget(ok, sc+3,1);
		setLayout(gridLayout);
		setWindowTitle(QString("Change Pixel Value"));
		
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		
		//slot interface
		connect(coord_x, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(coord_y, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(coord_z, SIGNAL(valueChanged(int)), this, SLOT(update()));
	}
	
	~ChangePxlValDialog(){}
	
public slots:
	void update();

		
public:
	unsigned char* data1d;
	V3DLONG N, M, P;
	V3DLONG sc;
	V3DLONG pagesz;
	
	QGridLayout *gridLayout;
	
	QLabel *labelx;
	QLabel *labely;
	QLabel *labelz;
	
	QSpinBox** nval;
	QLabel** nlabel;
	
	QSpinBox* coord_x;
	QSpinBox* coord_y;
	QSpinBox* coord_z;
	
	QPushButton* ok;
	QPushButton* cancel;
	
	V3DLONG nx,ny,nz;
	
};



#endif



