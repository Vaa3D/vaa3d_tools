/*
 * saveas_to_rawdata.h
 *
 *  Created by Yang, Jinzhu on 11/27/10.
 *  
 *
 */

#ifndef __SAVEAS_TO_RAWDATAlugin_H_
#define __SAVEAS_TO_RAWDATAlugin_H

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include "v3d_interface.h"


class SAVEAS_TO_RAWDATAlugin: public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface);
	
public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);
	
	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}	
	
};

void saveas_to_rawdata(V3DPluginCallback &callback, QWidget *parent, int method_code);

QHash<V3DLONG, V3DLONG> NeuronNextPn(const NeuronTree &neurons);


class SetsizeDialog : public QDialog
{
	Q_OBJECT
	
public:
	QGridLayout *gridLayout;
	
	QLabel *labelx;
	QLabel *labely;
	QLabel *labelz;
    QSpinBox* coord_x; 
	QSpinBox* coord_y;
	QSpinBox* coord_z;
	
	QPushButton* ok;
	QPushButton* cancel;
	
	V3DLONG NX;
	V3DLONG NY;
	V3DLONG NZ;
	
public:
	SetsizeDialog(V3DPluginCallback &cb, QWidget *parent)
	{
		Image4DSimple* image = cb.getImage(cb.currentImageWindow());
		QString imageName = cb.getImageName(cb.currentImageWindow());		
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
		
		gridLayout->addWidget(labelx, 0,0); gridLayout->addWidget(coord_x, 0,1);
		gridLayout->addWidget(labely, 1,0); gridLayout->addWidget(coord_y, 1,1);
		gridLayout->addWidget(labelz, 2,0); gridLayout->addWidget(coord_z, 2,1);
		
		gridLayout->addWidget(cancel, 6,1); gridLayout->addWidget(ok, 6,0);
		setLayout(gridLayout);
		setWindowTitle(QString("Change parameters"));
		
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


