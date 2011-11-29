/* imfill.h
 * 2010-04-02: create this program by Yang Yu
 */


#ifndef __IMFILL_H__
#define __IMFILL_H__


//
#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

#if defined (_MSC_VER)  //2010-05-25, by YUY for compatability to VC compiler
#include "../../basic_c_fun/vcdiff.h"
#else
#endif

#define fmin(x,y) ((x)>(y))?(y):(x) 


class ImFillPlugin : public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface);
	
public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);
	
	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}

};


class ImFillDialog : public QDialog
{
	Q_OBJECT
	
public:
	ImFillDialog(V3DPluginCallback &callback, QWidget *parent)
	{
		v3dhandleList win_list = callback.getImageWindowList();
		
		QStringList items;
		for (int i=0; i<win_list.size(); i++) items << callback.getImageName(win_list[i]);
		
		//create a dialog
		combo_subject =  new QComboBox(); combo_subject->addItems(items);
		
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		
		gridLayout = new QGridLayout();
		
		label_subject = new QLabel(QObject::tr("Choose An Image (with one marker): ")); 
		
		gridLayout->addWidget(label_subject, 0,0,1,1); gridLayout->addWidget(combo_subject, 0,1,1,4);

		i1 = combo_subject->currentIndex();
		
		Image4DSimple* subject = callback.getImage(win_list[i1]);
		
		int c = subject->getCDim();
		
		// channel info
		QStringList chList;
		
		if(c==3)
			chList << "red" << "green" << "blue";
		else if(c==2)
			chList << "red" << "green";
		else if(c==1)
			chList << "red";
		
		combo_channel = new QComboBox(); combo_channel->addItems(chList);
		
		label_channel = new QLabel(QObject::tr("Choose A Channel: ")); 
		
		gridLayout->addWidget(label_channel, 3,0,1,1); gridLayout->addWidget(combo_channel, 3,1,1,4);
		
		ch_rgb = combo_channel->currentIndex();
		
		//
		check = new QCheckBox();
		check->setText(QObject::tr("Choosing a background extraction method without setting a marker (if not checked, then use the mean value as a threshold)"));
		check->setChecked(false);
		
		gridLayout->addWidget(check, 5,0,1,4);
		
		// filling method
		label_background_thresh = new QLabel(QObject::tr("Set Background Threshold: "));	
		spinbox_background_thresh = new QSpinBox();
		spinbox_background_thresh->setMaximum(255); spinbox_background_thresh->setMinimum(0); spinbox_background_thresh->setValue(0); spinbox_background_thresh->setSingleStep(1);
		
		label_range = new QLabel(QObject::tr("Set an intensity range (+-): "));	
		spinbox_range = new QSpinBox();
		spinbox_range->setMaximum(255); spinbox_range->setMinimum(0); spinbox_range->setValue(3); spinbox_range->setSingleStep(1);
		
		gridLayout->addWidget(label_background_thresh, 7,0,1,1); gridLayout->addWidget(spinbox_background_thresh, 7,1,1,1);
		gridLayout->addWidget(label_range, 7,2,1,1); gridLayout->addWidget(spinbox_range, 7,3,1,1);
		
		mean_thresh = false;
		spinbox_background_thresh->setEnabled(false);
		spinbox_range->setEnabled(false);
		
		//gridlayout
		gridLayout->addWidget(cancel, 9,3,Qt::AlignRight); gridLayout->addWidget(ok, 9,4,Qt::AlignRight);
		
		setLayout(gridLayout);
		setWindowTitle(QString("Image Filling"));
		
		//slot interface
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		
		connect(check, SIGNAL(stateChanged(int)), this, SLOT(update()));
		
		connect(combo_subject, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));
		connect(combo_channel, SIGNAL(valueChanged(int)), this, SLOT(update()));
		
		connect(spinbox_background_thresh, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(spinbox_range, SIGNAL(valueChanged(int)), this, SLOT(update()));

		
	}
	
	~ImFillDialog(){}

	
public slots:
	void update()
	{
		i1 = combo_subject->currentIndex();
		ch_rgb = combo_channel->currentIndex();

		if(check->isChecked())
		{
			mean_thresh = true;
			spinbox_background_thresh->setEnabled(true);
			spinbox_range->setEnabled(true);
		}
		else
		{
			mean_thresh = false;
			spinbox_background_thresh->setEnabled(false);
			spinbox_range->setEnabled(false);
		}
		
		thresh = spinbox_background_thresh->text().toInt();
		range = spinbox_range->text().toInt();
	}
	
	
	
	
public:
	
	int i1;
	int ch_rgb;
	int thresh, range;
	bool mean_thresh; // false meanv true threshold
	
	QGridLayout *gridLayout;
	
	QLabel* label_subject;
	QComboBox* combo_subject;
	
	QLabel* label_channel;
	QComboBox* combo_channel;
	
	QCheckBox* check;
	
	QLabel* label_background_thresh;	
	QSpinBox* spinbox_background_thresh;
	
	QLabel* label_range;	
	QSpinBox* spinbox_range;
	
	QLabel* label_meanv;	
	QSpinBox* spinbox_meanv;
	
	QPushButton* ok;
	QPushButton* cancel;
	
};


#endif



