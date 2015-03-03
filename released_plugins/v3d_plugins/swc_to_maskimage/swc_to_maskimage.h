/*
 * swc_to_maskimg.h
 *
 *  Created by Yang, Jinzhu on 11/27/10.
 *  Last change: by Hanchuan Peng, 2010-Dec-7. change some writing style of this plugin header
 *
 */

#ifndef __SWC_TO_MASKIMAGE_H_
#define __SWC_TO_MASKIMAGE_H

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include "v3d_interface.h"

class SWC_TO_MASKIMAGElugin: public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);

public:
    float getPluginVersion() const {return 1.1f;}
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const;
     bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);

};

void swc_to_maskimage(V3DPluginCallback2 &callback, QWidget *parent, int method_code);
void mrskimage_originalimage(V3DPluginCallback2 &callback, QWidget *parent, int method_code);

void BoundNeuronCoordinates(NeuronTree & neuron,
							double & output_xmin,
							double & output_xmax,
							double & output_ymin,
							double & output_ymax,
							double & output_zmin,
							double & output_zmax
);

void ComputemaskImage(NeuronTree neurons,
					  unsigned char* pImMask, //output mask image
					  unsigned char* ImMark,  //an indicator image to show whether or not a pixel has been visited/processed
					  V3DLONG sx,
					  V3DLONG sy,
					  V3DLONG sz,
					  int method_code
);

QHash<V3DLONG, V3DLONG> NeuronNextPn(const NeuronTree &neurons);
void swc_to_maskimage_toolbox(const V3DPluginArgList & input, V3DPluginCallback2 & callback, QWidget * parent);
bool swc_to_maskimage(V3DPluginCallback2 & callback, const V3DPluginArgList & input, V3DPluginArgList & output);
void printHelp();


class SetsizeDialog : public QDialog
{
	Q_OBJECT

public:
	QGridLayout *gridLayout;

	QLabel *labelx;
	QLabel *labely;
	QLabel *labelz;
    QLabel *info;
    QSpinBox* coord_x;
	QSpinBox* coord_y;
	QSpinBox* coord_z;

	QPushButton* ok;
	QPushButton* cancel;

	V3DLONG NX;
	V3DLONG NY;
	V3DLONG NZ;

public:
	SetsizeDialog(V3DPluginCallback2 &cb, QWidget *parent)
	{
		Image4DSimple* image = cb.getImage(cb.currentImageWindow());
		QString imageName = cb.getImageName(cb.currentImageWindow());

        info= new QLabel;
        info->setText("The minimum dimensions of generated mask image are set as default.<br>"
                      " Please input new size. <br>"
                      "<b>Warning: size significantly below the default size will result in incomplete mask image<b> ");

		//create a dialog

		coord_x= new QSpinBox();
		coord_y = new QSpinBox();
		coord_z = new QSpinBox();

		coord_x->setMaximum(1500); coord_x->setMinimum(1); coord_x->setValue(11);
		coord_y->setMaximum(1500); coord_y->setMinimum(1); coord_y->setValue(11);
		coord_z->setMaximum(1500); coord_z->setMinimum(1); coord_z->setValue(11);

		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		gridLayout = new QGridLayout();

		labelx = new QLabel(QObject::tr("Image X dimension"));
		labely = new QLabel(QObject::tr("Image Y dimension"));
		labelz = new QLabel(QObject::tr("Image Z dimension"));

        info->setMargin(10);
        gridLayout->addWidget(info,0,0,4,2,Qt::AlignJustify);
        gridLayout->addWidget(labelx, 4,0,1,1,Qt::AlignHCenter); gridLayout->addWidget(coord_x,4,1,1,1,Qt::AlignHCenter);
        gridLayout->addWidget(labely, 5,0,1,1,Qt::AlignHCenter); gridLayout->addWidget(coord_y,5,1,1,1,Qt::AlignHCenter);
        gridLayout->addWidget(labelz, 6,0,1,1,Qt::AlignHCenter); gridLayout->addWidget(coord_z,6,1,1,1,Qt::AlignHCenter);

        gridLayout->addWidget(cancel, 7,1,1,1,Qt::AlignHCenter); gridLayout->addWidget(ok, 7,0,1,1,Qt::AlignHCenter);
		setLayout(gridLayout);
		setWindowTitle(QString("Change parameters"));
        //this->setFixedWidth(600);

		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

		//slot interface
		connect(coord_x, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(coord_y,SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(coord_z,SIGNAL(valueChanged(int)), this, SLOT(update()));
	}

	~SetsizeDialog(){}

	public slots:
	void update();
};
#endif


