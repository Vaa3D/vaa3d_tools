/* ITKShapeDetection.h
 * 2010-06-03: create this program by Yang Yu
 */

#ifndef __ITKSHAPEDETECTION_H__
#define __ITKSHAPEDETECTION_H__

//   Shape Detection.
//

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

class ITKShapeDetectionPlugin : public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface);
	
public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);
	
	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}
	
};

class ITKShapeDetectionDialog : public QDialog
{
	Q_OBJECT
	
public:
	ITKShapeDetectionDialog(V3DPluginCallback &callback, QWidget *parent)
	{
		v3dhandleList win_list = callback.getImageWindowList();
		
		QStringList items;
		for (int i=0; i<win_list.size(); i++) items << callback.getImageName(win_list[i]);
		
		// levelset
		label_subject = new QLabel(QObject::tr("Choose An Image (with at least one marker): ")); 
		combo_subject =  new QComboBox(); combo_subject->addItems(items);
		
		// when no image open return
		if(win_list.size()<1) 
		{
			QMessageBox::information(0, "Shape Detection", QObject::tr("No image is open."));
			return;
		}
		
		printf("Passing data to data1d\n");
		
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		
		//gridlayout
		gridLayout = new QGridLayout();
		
		gridLayout->addWidget(label_subject, 0,0,1,1); gridLayout->addWidget(combo_subject, 0,1,1,4);
		gridLayout->addWidget(cancel, 2,4); gridLayout->addWidget(ok, 2,5);
		setLayout(gridLayout);
		setWindowTitle(QString("Shape Detection"));
		
		//slot
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		
		connect(combo_subject, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));
	}
	
	~ITKShapeDetectionDialog(){}
	
public slots:
	void update()
	{
		i1 = combo_subject->currentIndex();
	}
	
	
public:
	
	int i1;
	
	QGridLayout *gridLayout;
	
	QLabel* label_subject;
	QComboBox* combo_subject;
	
	QPushButton* ok;
	QPushButton* cancel;
};


#endif
