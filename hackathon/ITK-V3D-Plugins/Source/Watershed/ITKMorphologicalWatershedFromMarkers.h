/* ITKMorphologicalWatershedFromMarkers.h
 * 2010-06-04: create this program by Yang Yu
 */

#ifndef __ITKMORPHOLOGICALWATERSHEDFROMMARKERS_H__
#define __ITKMORPHOLOGICALWATERSHEDFROMMARKERS_H__

//   Morphological Watershed From Markers.
//

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

class ITKMorphologicalWatershedFromMarkersPlugin : public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface);
	
public:
	ITKMorphologicalWatershedFromMarkersPlugin() {}
	QStringList menulist() const;
	QStringList funclist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);
	
	virtual void dofunc(const QString & func_name,
						const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent);
	
};

class ITKMorphologicalWatershedFromMarkersDialog : public QDialog
{
	Q_OBJECT
	
public:
	ITKMorphologicalWatershedFromMarkersDialog(V3DPluginCallback &callback, QWidget *parent)
	{
		v3dhandleList win_list = callback.getImageWindowList();
		
		QStringList items;
		for (int i=0; i<win_list.size(); i++) items << callback.getImageName(win_list[i]);
		
		// image
		label_subject = new QLabel(QObject::tr("Choose An Image: ")); 
		combo_subject =  new QComboBox(); combo_subject->addItems(items);
		
		// speed 
		label_maker = new QLabel(QObject::tr("Choose A Makers Image: ")); 
		combo_marker =  new QComboBox(); combo_marker->addItems(items);
		
		// when no image open return
		if(win_list.size()<1) 
		{
			QMessageBox::information(0, "Watershed Segmentation From Makers", QObject::tr("No image is open."));
			return;
		}
		
		printf("Passing data to data1d\n");
		
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		
		//gridlayout
		gridLayout = new QGridLayout();
		
		gridLayout->addWidget(label_subject, 0,0,1,1); gridLayout->addWidget(combo_subject, 0,1,1,4);
		gridLayout->addWidget(label_maker, 2,0,1,1); gridLayout->addWidget(combo_marker, 2,1,1,4);
		gridLayout->addWidget(cancel, 4,4); gridLayout->addWidget(ok, 4,5);
		setLayout(gridLayout);
		setWindowTitle(QString("Watershed Segmentation From Makers"));
		
		//slot
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		
		connect(combo_subject, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));
		connect(combo_marker, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));
	}
	
	~ITKMorphologicalWatershedFromMarkersDialog(){}
	
public slots:
	void update()
	{
		i1 = combo_subject->currentIndex();
		i2 = combo_marker->currentIndex();
	}
	
public:
	int i1,i2;
	
	QGridLayout *gridLayout;
	
	QLabel* label_subject;
	QComboBox* combo_subject;
	
	QLabel* label_maker;
	QComboBox* combo_marker;
	
	QPushButton* ok;
	QPushButton* cancel;
};


#endif
