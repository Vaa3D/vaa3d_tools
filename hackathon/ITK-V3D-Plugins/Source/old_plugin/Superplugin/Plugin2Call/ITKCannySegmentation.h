/* ITKCannySegmentation.h
 * 2010-06-04: create this program by Yang Yu
 */

#ifndef __ITKCANNYSEGMENTATION_H__
#define __ITKCANNYSEGMENTATION_H__

//   CANNY SEGMENTATION.
//

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

class ITKCannySegmentationPlugin : public QObject, public V3DPluginInterface2
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2);
	
public:
	ITKCannySegmentationPlugin () {}
	QStringList menulist() const;
	QStringList funclist() const;

	void domenu(const QString & menu_name, V3DPluginCallback2 & callback, QWidget * parent);

	bool dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
				 V3DPluginCallback2 & v3d,  QWidget * parent);

	
};

class ITKCannySegmentationDialog : public QDialog
{
	Q_OBJECT
	
public:
	ITKCannySegmentationDialog(V3DPluginCallback &callback, QWidget *parent)
	{
		v3dhandleList win_list = callback.getImageWindowList();
		
		QStringList items;
		for (int i=0; i<win_list.size(); i++) items << callback.getImageName(win_list[i]);
		
		// image
		label_subject = new QLabel(QObject::tr("Choose An Image: ")); 
		combo_subject =  new QComboBox(); combo_subject->addItems(items);
		
		// speed 
		label_model = new QLabel(QObject::tr("Choose A Model Image: ")); 
		combo_model =  new QComboBox(); combo_model->addItems(items);
		
		// when no image open return
		if(win_list.size()<1) 
		{
			QMessageBox::information(0, "Canny Segmentation", QObject::tr("No image is open."));
			return;
		}
		
		printf("Passing data to data1d\n");
		
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		
		//gridlayout
		gridLayout = new QGridLayout();
		
		gridLayout->addWidget(label_subject, 0,0,1,1); gridLayout->addWidget(combo_subject, 0,1,1,4);
		gridLayout->addWidget(label_model, 2,0,1,1); gridLayout->addWidget(combo_model, 2,1,1,4);
		gridLayout->addWidget(cancel, 4,4); gridLayout->addWidget(ok, 4,5);
		setLayout(gridLayout);
		setWindowTitle(QString("Canny Segmentation"));
		
		//slot
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		
		connect(combo_subject, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));
		connect(combo_model, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));
	}
	
	~ITKCannySegmentationDialog(){}
	
public slots:
	void update()
	{
		i1 = combo_subject->currentIndex();
		i2 = combo_model->currentIndex();
	}
	
public:
	int i1,i2;
	
	QGridLayout *gridLayout;
	
	QLabel* label_subject;
	QComboBox* combo_subject;
	
	QLabel* label_model;
	QComboBox* combo_model;
	
	QPushButton* ok;
	QPushButton* cancel;
};


#endif
